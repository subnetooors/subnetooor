
#include "abcinetserver.h"
#include "abcihandler.h"
#include "abcinetsession.h"

#include "../../utils/logger.h"

// TODO: review all the error handling and the net code in general

ABCINetSession::ABCINetSession(ABCIHandler *handler, boost::asio::local::stream_protocol::socket socket, std::shared_ptr<ABCINetServer> server)
  : handler_(handler),
    socket_(std::move(socket)),
    strand_(socket_.get_executor()),
    server_(server)
{
}

void ABCINetSession::start() {
  boost::asio::post(
    strand_,
    std::bind(&ABCINetSession::start_read_varint, shared_from_this())
  );
}

void ABCINetSession::close() {
  boost::asio::dispatch(
    boost::asio::bind_executor(
      strand_,
      std::bind(&ABCINetSession::do_close, shared_from_this())
    )
  );
}

void ABCINetSession::do_close() {
  if (closed_) {
    return;
  }

  boost::system::error_code ec;
  socket_.cancel(ec);
  if (ec) {
    LOGTRACE("Failed to cancel socket operations: " + ec.message());
  }

  socket_.close(ec);
  if (ec) {
    LOGTRACE("Failed to close socket: " + ec.message());
  }

  LOGXTRACE("Closed socket");
  closed_ = true;
}

void ABCINetSession::start_read_varint() {
  varint_value_ = 0;
  varint_shift_ = 0;

  boost::asio::post(
    strand_,
    std::bind(&ABCINetSession::start_read_varint_byte, shared_from_this())
  );
}

void ABCINetSession::start_read_varint_byte() {
  boost::asio::async_read(
    socket_,
    boost::asio::buffer(&varint_byte_, 1),
    boost::asio::bind_executor(
      strand_,
      std::bind(&ABCINetSession::handle_read_varint_byte, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
    )
  );
}

void ABCINetSession::handle_read_varint_byte(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    server_->stop("Error reading varint byte: " + ec.message());
    return;
  }

  varint_value_ |= ((uint64_t)(varint_byte_ & 0x7F)) << varint_shift_;
  if (!(varint_byte_ & 0x80)) {
    handle_read_message_length(true, varint_value_);
  } else {
    varint_shift_ += 7;
    if (varint_shift_ >= 64) {
      server_->stop("Varint too long");
      return;
    }
    start_read_varint_byte();
  }
}

void ABCINetSession::handle_read_message_length(bool success, uint64_t msg_len) {
  if (!success || msg_len == 0) {
    if (!success) {
      server_->stop("Error reading message length (failed)");
    } else {
      server_->stop("Error reading message length (len==0)");
    }
    return;
  }

  LOGDEBUG("WILL READ MESSAGE OF SIZE: " + std::to_string(msg_len));
  message_data_.resize(msg_len);

  boost::asio::async_read(
    socket_,
    boost::asio::buffer(message_data_),
    boost::asio::bind_executor(
      strand_,
      std::bind(&ABCINetSession::handle_read_message, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
    )
  );
}

void ABCINetSession::handle_read_message(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    server_->stop("Error reading message data: " + ec.message());
    return;
  }
  process_request();
}

void ABCINetSession::do_write_message() {
  // Prepare the varint
  varint_buffer_.clear();
  write_varint(response_data_.size(), varint_buffer_);

  LOGDEBUG("WILL WRITE VARINT OF SIZE: " + std::to_string(varint_buffer_.size()) + ", FOLLOWED BY MESSAGE OF SIZE: " + std::to_string(response_data_.size()));

  // Write the varint first
  boost::asio::async_write(
    socket_,
    boost::asio::buffer(varint_buffer_),
    boost::asio::bind_executor(
      strand_,
      std::bind(&ABCINetSession::handle_write_varint, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
    )
  );
}

void ABCINetSession::handle_write_varint(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    server_->stop("Error writing varint: " + ec.message());
    return;
  }

  LOGDEBUG("VARINT WRITTEN, SIZE: " + std::to_string(length));

  // Write the actual response buffer next
  boost::asio::async_write(
    socket_,
    boost::asio::buffer(response_data_),
    boost::asio::bind_executor(
      strand_,
      std::bind(&ABCINetSession::handle_write_message, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
    )
  );
}

void ABCINetSession::handle_write_message(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    server_->stop("Error writing response: " + ec.message());
    return;
  }

  LOGDEBUG("MESSAGE WRITTEN, SIZE: " + std::to_string(length));

  // Proceed to read the next message
  boost::asio::post(
    strand_,
    std::bind(&ABCINetSession::start_read_varint, shared_from_this())
  );
}

void ABCINetSession::write_varint(uint64_t value, std::vector<uint8_t> &buffer) {
  while (true) {
    uint8_t byte = value & 0x7F;
    value >>= 7;
    if (value) {
      byte |= 0x80;
    }
    buffer.push_back(byte);
    if (!value) {
      break;
    }
  }
}

void ABCINetSession::process_request() {
  // Parse the message into a Request
  cometbft::abci::v1::Request request;
  if (!request.ParseFromArray(message_data_.data(), message_data_.size())) {
    server_->stop("Failed to parse request");
    return;
  }

  // Create a Response message
  cometbft::abci::v1::Response response;

  // Handle the request
  switch (request.value_case()) {
    case cometbft::abci::v1::Request::kEcho:
    {
      LOGXTRACE("Echo");
      const auto &echo_req = request.echo();
      auto *echo_resp = response.mutable_echo();
      // Actually set the correct/expected echo answer here.
      echo_resp->set_message(echo_req.message());
      // The caller doesn't actually have to do anything.
      handler_->echo(echo_req, echo_resp);
      break;
    }
    case cometbft::abci::v1::Request::kFlush:
    {
      LOGXTRACE("Flush");
      const auto &flush_req = request.flush();
      auto *flush_resp = response.mutable_flush();
      handler_->flush(flush_req, flush_resp);
      break;
    }
    case cometbft::abci::v1::Request::kInfo:
    {
      LOGXTRACE("Info");
      const auto &info_req = request.info();
      auto *info_resp = response.mutable_info();
      handler_->info(info_req, info_resp);
      break;
    }
    case cometbft::abci::v1::Request::kInitChain:
    {
      LOGXTRACE("InitChain");
      const auto &init_chain_req = request.init_chain();
      auto *init_chain_resp = response.mutable_init_chain();
      handler_->init_chain(init_chain_req, init_chain_resp);
      break;
    }
    case cometbft::abci::v1::Request::kPrepareProposal:
    {
      LOGXTRACE("PrepareProposal");
      const auto &prepare_proposal_req = request.prepare_proposal();
      auto *prepare_proposal_resp = response.mutable_prepare_proposal();
      handler_->prepare_proposal(prepare_proposal_req, prepare_proposal_resp);
      break;
    }
    case cometbft::abci::v1::Request::kProcessProposal:
    {
      LOGXTRACE("ProcessProposal");
      const auto &process_proposal_req = request.process_proposal();
      auto *process_proposal_resp = response.mutable_process_proposal();
      handler_->process_proposal(process_proposal_req, process_proposal_resp);
      break;
    }
    case cometbft::abci::v1::Request::kCheckTx:
    {
      LOGXTRACE("CheckTx");
      const auto &check_tx_req = request.check_tx();
      auto *check_tx_resp = response.mutable_check_tx();
      handler_->check_tx(check_tx_req, check_tx_resp);
      break;
    }
    case cometbft::abci::v1::Request::kQuery:
    {
      LOGXTRACE("Query");
      const auto &query_req = request.query();
      auto *query_resp = response.mutable_query();
      handler_->query(query_req, query_resp);
      break;
    }
    case cometbft::abci::v1::Request::kCommit:
    {
      LOGXTRACE("Commit");
      const auto &commit_req = request.commit();
      auto *commit_resp = response.mutable_commit();
      handler_->commit(commit_req, commit_resp);
      break;
    }
    case cometbft::abci::v1::Request::kExtendVote:
    {
      LOGXTRACE("ExtendVote");
      const auto &extend_vote_req = request.extend_vote();
      auto *extend_vote_resp = response.mutable_extend_vote();
      handler_->extend_vote(extend_vote_req, extend_vote_resp);
      break;
    }
    case cometbft::abci::v1::Request::kVerifyVoteExtension:
    {
      LOGXTRACE("VerifyVoteExtension");
      const auto &verify_vote_extension_req = request.verify_vote_extension();
      auto *verify_vote_extension_resp = response.mutable_verify_vote_extension();
      handler_->verify_vote_extension(verify_vote_extension_req, verify_vote_extension_resp);
      break;
    }
    case cometbft::abci::v1::Request::kFinalizeBlock:
    {
      LOGXTRACE("FinalizeBlock");
      const auto &finalize_block_req = request.finalize_block();
      auto *finalize_block_resp = response.mutable_finalize_block();
      handler_->finalize_block(finalize_block_req, finalize_block_resp);
      break;
    }
    case cometbft::abci::v1::Request::kListSnapshots:
    {
      LOGXTRACE("ListSnapshots");
      const auto &list_snapshots_req = request.list_snapshots();
      auto *list_snapshots_resp = response.mutable_list_snapshots();
      handler_->list_snapshots(list_snapshots_req, list_snapshots_resp);
      break;
    }
    case cometbft::abci::v1::Request::kOfferSnapshot:
    {
      LOGXTRACE("OfferSnapshot");
      const auto &offer_snapshot_req = request.offer_snapshot();
      auto *offer_snapshot_resp = response.mutable_offer_snapshot();
      handler_->offer_snapshot(offer_snapshot_req, offer_snapshot_resp);
      break;
    }
    case cometbft::abci::v1::Request::kLoadSnapshotChunk:
    {
      LOGXTRACE("LoadSnapshotChunk");
      const auto &load_snapshot_chunk_req = request.load_snapshot_chunk();
      auto *load_snapshot_chunk_resp = response.mutable_load_snapshot_chunk();
      handler_->load_snapshot_chunk(load_snapshot_chunk_req, load_snapshot_chunk_resp);
      break;
    }
    case cometbft::abci::v1::Request::kApplySnapshotChunk:
    {
      LOGXTRACE("ApplySnapshotChunk");
      const auto &apply_snapshot_chunk_req = request.apply_snapshot_chunk();
      auto *apply_snapshot_chunk_resp = response.mutable_apply_snapshot_chunk();
      handler_->apply_snapshot_chunk(apply_snapshot_chunk_req, apply_snapshot_chunk_resp);
      break;
    }
    default:
    {
      LOGXTRACE("Unknown Request Type (ERROR)");
      server_->stop("Received an unknown request type");
      auto *exception_resp = response.mutable_exception();
      exception_resp->set_error("Unknown request type");
      break;
    }
  }

  // Serialize the response
  size_t response_size = response.ByteSizeLong();
  response_data_.resize(response_size);
  if (!response.SerializeToArray(response_data_.data(), response_size)) {
    server_->stop("Failed to serialize response");
    return;
  }

  // Write the response
  do_write_message();
}
