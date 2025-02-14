/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/merkle.h" // tx.h
#include "utils/strconv.h"

#include "bytes/random.h"

using Catch::Matchers::Equals;

namespace TMerkle {
  TEST_CASE("Merkle Tests", "[unit][utils][merkle]") {
    SECTION("Simple Merkle Tree") {
      std::vector<Bytes> unhashedLeafs = {
        StrConv::stringToBytes("ab"), StrConv::stringToBytes("bc"), StrConv::stringToBytes("cd"),
        StrConv::stringToBytes("de"), StrConv::stringToBytes("ef"), StrConv::stringToBytes("fg"),
        StrConv::stringToBytes("gh"), StrConv::stringToBytes("hi"), StrConv::stringToBytes("ij"),
        StrConv::stringToBytes("jk"), StrConv::stringToBytes("km"), StrConv::stringToBytes("mn")
      };
      std::vector<Hash> hashedLeafs;
      for (const Bytes& leaf : unhashedLeafs) hashedLeafs.emplace_back(Utils::sha3(leaf));

      Merkle tree(hashedLeafs);
      std::vector<Hash> proof = tree.getProof(3);
      Hash leaf = tree.getLeaves()[3];
      Hash root = tree.getRoot();
      Hash badLeaf = tree.getLeaves()[4];

      REQUIRE_THAT(root.hex(), Equals("3fb0308018d8a6b4c2081699003624e9719774be2b7f65b7f9ac45f2bebc20b7"));
      REQUIRE(Merkle::verify(proof, leaf, root));
      REQUIRE(!Merkle::verify(proof, badLeaf, root));

      // For coverage
      REQUIRE(tree.getProof(999).empty());
    }

    SECTION("Random Merkle Tree") {
      std::vector<Hash> hashedLeafs {
        bytes::random(), bytes::random(), bytes::random(), bytes::random(),
        bytes::random(), bytes::random(), bytes::random(), bytes::random(),
        bytes::random(), bytes::random(), bytes::random(), bytes::random(),
        bytes::random(), bytes::random(), bytes::random()
      };

      Merkle tree(hashedLeafs);
      std::vector<Hash> proof = tree.getProof(5);
      Hash leaf = tree.getLeaves()[5];
      Hash root = tree.getRoot();
      Hash badLeaf = tree.getLeaves()[6];

      REQUIRE(Merkle::verify(proof, leaf, root));
      REQUIRE(!Merkle::verify(proof, badLeaf, root));
    }

    SECTION("TxBlock Merkle Tree") {
      std::vector<TxBlock> txs {
        TxBlock(Hex::toBytes("02f8f2048697292f15c01784020833138605bde3949c038407b5432c94dd9bada36d88dac984e6d10a96061b65bc034a668617521be83d4cb8776129345086fa05e37c56ea3a9a24e102f3519b92f16f8f20281dcb28ef36eef04dd23f355693a24c4ce7de4ae663bfd5a47670092efefde1e612ff6fb380e3fe272121fb4c689454edb24d7287a8e4797f85934ab9514d20e77fb7a7c6a7e4921c614934ea2bb26737c34d8995a6d5505644d020f16d6ac001a0c806640faf85b1d0d1e5ffce56e037e7cc7d5e852cbc02e5c7318629de9eb3daa024c9e1bd51bb961dce5bb25ee834129f51e2f8c23fcee796b388cced5cae170e"), 1),
        TxBlock(Hex::toBytes("02f8fc851c87806f948701960cc0c1e3f98568b0f2323f87021b8b1bcd405e881ab17ab60daa62e794583d6b8bf349f56e270915bebd69ddb7b7dba71387fb652d0e3308d5b874b25155d71f72ef6b8dce2b77f841f5eb9f36843ecebafd4f41400c905c67d67237243f91a25fbdbfbd1825709bffe3f4e402475534891ae1648247cf8784d1d3af12d49b3b73d5e65498c70eee4061e434958cf5ce81d7b7fe466d33f4ed371aa48d1e3140eebc8dbb961994dfdf4a03554a48efc080a02cdc0a9bf48c958421df1878a08f2126ebf63a96df4de4ba4f7e66515498bae9a0246674c3c3962300b9f164d035ee3c966d4a27452b792fea2289989126fc7ab1"), 1),
        TxBlock(Hex::toBytes("02f8ca820d148811173620932395ca88055e717c0de4ceb088030541a706491b398704659a74ec01e79403ab524baf9055f8eb6cbdef3cc046293b26ab5a89022c84c915bc220a71b83f27957117aca3bd31050081fc1bca3f832af0f67e5089cbd529d6084b02ea9a0590a48d96b9f54b50c5951633e4e986a21c0022020d7b5b90784a21fd331339c001a00e2f853498eeae502dc5e83f4c322cd84c677f460b2e3664ed855b7234e3e0d3a04b7ea025cdde7cb150ef5332f3a9c8c1d4e316a17663d00bf77c877eb905fb70"), 1),
        TxBlock(Hex::toBytes("02f8f27f8549db7390cd82fa6487ce5bc0bbb6789e86af13798cda7694d73cdabcec0cc182de86b4c012ce243ebbc3ab4b833e2f1eb87abe3ca20e4c8698bf768eeb743dd5217ce55075396f34b97a9d600b5f4bd8562867150e7e2976193b5b7134b7bc834148dc8d62f82547272a5573bfed8b52ff4b904c2e38b6b6a43814ee33d2e671f6fc533daacd3af259f286af3a77c73e49a4016a01b48d9e6a3a3ab2644457d91870b93ca32384fc3c86ad69c001a0a3fd1498bfa6e5e13cce3e5768aec5aac1a912b42dd6751ddb16f6324975c452a074c377ce7a318505df2c4c06e4935fef1e61562561ebcae276f7b4c2891c387c"), 1),
        TxBlock(Hex::toBytes("02f8be1885e795e6d8e680808094c6ff22a04cc9ed476eede000eb0ac51661225bde8e1f31a323866448f957c82f1f84cbb84a1fa09bf8d8cd7ce5f37fd25dad940e9623433d1c4916b8d2f43240b3e38c1de579e2b064f7f0b0ec66af28c8a8f4ed19dfc033540e37a1ff95e5f5d95b7f3fc48922ab488e008f96e521c001a0358ed0ac0aba663f426f70ba43378e338fd615a9c91321d843802523a7dd63a3a068fe62a2241678088b1648d4e951c6d0aac64726e0c8798a5720a04fe0b3b94f"), 1),
        TxBlock(Hex::toBytes("02f8c4837cdd14837ee7fc83e2cb8387654e4393c0c1e7865383660762bb94f3fd73725fe9dbc1c83b734ee624e3722d1045088c70cf0ce638d2b0e9b167cf3bb841d798df4008c491d5fd4109efc3c69fcbea65857915bbcecf8b2c7f35ed364df117ef2892c74a84d658aace2e5b9f3f95c27fbcdfb7ef32cd778b91ca788c60f5bbc001a0dbfd187d4b215d4a9a673d0887214a7d7dac2eea4a0c7a1aebef5dd4be7d4d8ba01e877b9fdeba11f54229f85ec3d819155539a7c61c2e8a13e58dbf745ad53724"), 1),
        TxBlock(Hex::toBytes("02f8dd81b283554a1486315b9a3bcd2c808094c51c568dc73472818ef69bfdb3fb88a4413e2c2a881e0315140a1de529b86a641153d6d7aa0fbd6b04cd10e096f0d6e08f05ef94f1f9298cb9e64bbd6c8511fa03c705db9a51c64b5b2f327198880a477c1d4f6a03b000bdaa0a72a129eca78f0aaad2a6a2cdf7a79b50f89a49903df3f60db2801ed1159d0ace69843211b8554ebcbc1ab832965f04c001a0f7d8ccd52d8542e1dde78e39c09b064628b16acb2fb5fa5b242296203a3c3363a020870c0e321fa2fe79c08894eb536a580012389b3a5d2b82c023b9978d723965"), 1),
        TxBlock(Hex::toBytes("02f8d583522a60868c776d3ab2ed859d31919a0581dc82022b9423d1a55e919efb2ec8f5c12dc41f0c4c1c1dba6b80b86366136b47795c389b015845e6e5b07b6405c37fe21f4be1535db3479f62d8c62b9c7ef715513edc407eb882448c199df84440461d7e1108e17d57ebf3eed8abdf1f68e64787261bf1438432e1435ce7911e54a2d216d811ba0c6d450a96dfcbe4ec3287c080a0e9fcedbbfd788120017f6b1233c9001f83602cb045ee256055265c87dc8bc99fa00d7b6c83524644905974974f05d6d0d48a32506d97a26d7d2b7154780c8642ee"), 1),
        TxBlock(Hex::toBytes("02f8f183fecd308737706951d3968c8553cb1bcefb8203cf019493725531e3371e2b0baeb1f75c5b5102c04bf4438be656bacbbc8d9c4ef82cd8b874bf2ac52475bb963dcc43d6f10349a55e1f30899461e358e0adaa237bb6b9afe845aaffea7087eda3f3c38b81f167fa694c775c33897768da59f74552ec800afe193b87e8443b812c154c690e9b30eda40adad171c0dfbfb5c8c97c755583a7fdf06a8377a1e2dc9a97eb8709660d0fee6a09dfb8c001a0d45b39198384c352ceefc1f23960a83f0603724d9267702e9a189b0a42a5b65aa07d22367eb8b6ddae1d5ecb63b836e9eb296e675f344c35bd329aa09a4c9fced9"), 1),
        TxBlock(Hex::toBytes("02f8dc849f58fd12868dc1979857ac8190800194912bd947f01f8324bbc2f22348f09df0b0bb077f80b870b41dff6f4be0db9f6ed217bb7955123ba9cbe30468e5f152a4273f3b483b752834c8bdd97127047e43fe8d117cffbd31a1b28481623bf9fbae09a86c024316af618e04cf896f9b37eac968c051da82f3b3757d9d7dca88eed7dd959abbf1154b64c3c22a2c3e92360ee02c05ff6b1af4c080a0b2bb4b8df4b6c371872f634782eefc50afe2fb2c975758afc12b734bd70630a5a030fc638b881b37cd4f95bd1372a6a3214daca59eba4c45cfee0001c0c4bf1ad5"), 1),
        TxBlock(Hex::toBytes("02f8e285f72e314406863b966f67db1e82dd5980851cda1d825d943a8970efe474c3770f7a6f4d68fcd2d84a5444ee87b13dd3fbf54530b868d4210fb5fe6b195ebc558373e7f724aecc3eeb0a4c852118bf61016f1bf2d8fb1da0b946790ac12cc009a4fde91d96833d5641054abb9bab3ac0f7c2c01c24ef9ba1c5d1c7a58b1e8ade7f06a6ac33a3ce95d66f082bc587900379b7042ccefe1e501ad2202c39dac001a0b2eeb75401d31d7830e63eb80ae1a12dad1fb4a24488f20d99078ebbcb62cae6a00b2e716fd387203bc019a1b6747994a0859dcabf035fbc77231a8b20147f3634"), 1),
        TxBlock(Hex::toBytes("02f89284e8fd350765808080948d65b2a2437da6a0ff1e575a75f8f4fe22d056ea87ab4202e0fad64aa7bea2c51b92c070a2ed7d78e3ccc3969d32b6d4db8d5af381f84a7c7dd9b4189872d328a189b653c001a0d0bf5f1867ee4d765cc71a0e8b1bf048ebc9c4c8cf6306dc9788dbf6bb4d8739a04e70cc3431f1d1fcdf89921833854f4a2572787cd8603bb4d84cf32638bcbe91"), 1),
        TxBlock(Hex::toBytes("02f8a1853c0cfe560c85406a1f50f185ab87453de883cfa35d851e1fbe358b94f13174546ec556878140dd71b46f6f4da9e3199380aa0238ac252b052fffc4fbe937b2d2f177f4bb71ed6fdfd5db7aec43c6ca517354f9e40845ac8c885a9b8ac080a0458e8e0d37f0759d7ec77e5b8d7baebb2a36982d973f51cb3520f4f642f32653a06ffd5813a99629a3f5a91e3b6d6b79017aea5c14990d71154f642df2ce5c9379"), 1),
        TxBlock(Hex::toBytes("02f8f0833ae30187e5223863e60ecf868fc8332be5d2856720f8a4c98652fb1de41e2b94fc3106d6242b762c37f21c021395dbfdc9a2c0d08b5fd95952973e70427608c0b869bf32ad6cc8188ff0ac6a14727bad727dcdec7f4b17a8b99499946f386d38a69c1945077f7b9a199a2b6261f37700affb3c334e9e2ff2f088ab47f407559920e0e18e565dab9c6af48fffc3b2f49db7e0175ec32bfc30c542753e7f299ef8cfe3c3b3b555a937b6bff6c001a0ba0847339c8c3ec70c17bade31baaa1693804020af9f787df8e876685d24d828a057e3f9a559f3b1f07f9afdd4417c1f8082029ec8ed9b9b21fbd87a0e4b863722"), 1),
        TxBlock(Hex::toBytes("02f8c2845e3a9a641c8585f1aa842c80852ce60d0c44942b990b906b7fda0e388fcce3c0a93324f2bd57a789e7ca74912e80664d8bb84a13e6cf536ac96d352af5272e7ea1ef859e13b79405fcd6237f1119dfedf8452de41f9ea1b262234272b67566e5bb0d91e7e8bf978c461029533cd447058e4b74e631365f439eecf6b14ec001a0ba2cd19a7429eb743cc07699fab6d447deb8b684877850437e3139f861dd892aa050710399a557eb7df836b2a838532d85ce5a8e2d1f1cb2045b9cab3b7e1bcf1c"), 1),
        TxBlock(Hex::toBytes("02f89e833fd7e3862ebf6a27942f81bc80838c1cb5940ec60eb0866819b94f347b5bb651c5129f988f5921b1887c6c6bdf2df9f0ba4c9d77a5478b86b442c4a1a7f48e64951023813eb49e1995f42b0f7362e2308933ce6f46020f6469c080a05f1bafe4f6e042c9908e931f81616bad4b093e6d394ded9334f2345910167afea02fd084070e214f47d81d18724ed1183a9a4f1f4219c3ee783972838e0fd11e7d"), 1)
      };

      Merkle tree(txs);
      std::vector<Hash> proof = tree.getProof(3);
      Hash leaf = tree.getLeaves()[3];
      Hash root = tree.getRoot();
      Hash badLeaf = tree.getLeaves()[4];

      REQUIRE(root == Hash(Hex::toBytes("218b07ea5b9cc6f18abc6d032b568da8d7b984060ad5c0f90472c422f9db986b")));
      REQUIRE(Merkle::verify(proof, leaf, root));
      REQUIRE(!Merkle::verify(proof, badLeaf, root));
    }
  }
}

