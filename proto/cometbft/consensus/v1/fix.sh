#!/bin/bash

# --------------------------------------------------------------------------------
# Script run after consensus/v1/types.pb.cc and .h are generated to fix them due
#   to vote / has_vote fields in types.proto generating symbol collisions.
#
# TODO: fully validate; not sure if this is enough to make it work.
# --------------------------------------------------------------------------------

# Determine the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"



# Define the header file path
HEADER_FILE="${SCRIPT_DIR}/types.pb.h"

# Check if the header file exists
if [ ! -f "$HEADER_FILE" ]; then
  echo "Header file $HEADER_FILE not found!"
  exit 1
fi

# First substitution: Replace `bool has_vote() const;` with `bool MANGLED_has_vote() const;`
sed -i '/\/\/ .cometbft.consensus.v1.Vote vote = 6;/,+1s|bool has_vote() const;|bool MANGLED_has_vote() const;|' "$HEADER_FILE"

# Second substitution: Replace `bool _internal_has_vote() const;` with `bool _internal_MANGLED_has_vote() const;`
sed -i '/\/\/ .cometbft.consensus.v1.Vote vote = 6;/,+3s|bool _internal_has_vote() const;|bool _internal_MANGLED_has_vote() const;|' "$HEADER_FILE"

# Third substitution: Replace `inline bool Message::_internal_has_vote() const {` with `inline bool Message::MANGLED_internal_has_vote() const {`
sed -i '/\/\/ .cometbft.consensus.v1.Vote vote = 6;/,+5s|inline bool Message::_internal_has_vote() const {|inline bool Message::_internal_MANGLED_has_vote() const {|' "$HEADER_FILE"

# Fourth substitution: Replace `return _internal_has_vote();` with `return MANGLED_internal_has_vote();`
sed -i '/\/\/ .cometbft.consensus.v1.Vote vote = 6;/,+6s|return _internal_has_vote();|return _internal_MANGLED_has_vote();|' "$HEADER_FILE"

# Fifth substitution: Replace `inline bool Message::has_vote() const {` with `inline bool Message::MANGLED_has_vote() const {`
sed -i '/\/\/ .cometbft.consensus.v1.Vote vote = 6;/,+7s|inline bool Message::has_vote() const {|inline bool Message::MANGLED_has_vote() const {|' "$HEADER_FILE"

# Global substitution: Replace all `_internal_has_vote())` with `_internal_MANGLED_has_vote())`
sed -i 's|_internal_has_vote())|_internal_MANGLED_has_vote())|g' "$HEADER_FILE"

# 
sed -i '/inline const ::cometbft::consensus::v1::Vote& Message::_internal_vote() const {/,+1s|return _internal_has_vote()|return _internal_MANGLED_has_vote()|' "$HEADER_FILE"

echo "Substitutions complete in $HEADER_FILE."




# Define the .cc file path
CC_FILE="${SCRIPT_DIR}/types.pb.cc"

# Check if the .cc file exists
if [ ! -f "$CC_FILE" ]; then
  echo "CC file $CC_FILE not found!"
  exit 1
fi

# First substitution: Replace `_internal_has_vote()` with `_internal_MANGLED_has_vote()` in the specific context
sed -i '/\/\/ .cometbft.consensus.v1.Vote vote = 6;/,+1s|_internal_has_vote()|_internal_MANGLED_has_vote()|' "$CC_FILE"

echo "Substitution complete in $CC_FILE."
