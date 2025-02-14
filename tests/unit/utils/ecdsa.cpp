/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/ecdsa.h" // utils.h

using Catch::Matchers::Equals;

namespace TSecp256k1 {
  TEST_CASE("Secp256k1 Tests", "[unit][utils][secp256k1]") {
    SECTION("Recover Public Key from Signature") {
      Hash msg = Utils::sha3(Utils::create_view_span(std::string("Hello World!")));
      Signature sig(Hex::toBytes("e7a9dc85504bf4f79732e55c25fded4dd5471dfc28a6e35463aab7e8dfb180b5414520f0e8f18ec174fc2e14ce4c98f12faf58344c00af87c50b7bc502ac6b5f01"));
      UPubKey uPubKey = Secp256k1::recover(sig, msg);
      auto address = Secp256k1::toAddress(uPubKey);
      REQUIRE_THAT(uPubKey.hex(), Equals("0431212407a958f50d1b7ee2bf0c44ad2e01090a917660f71bf5b41f470026d3a584bfbc977bbdf9b82b5473fdfabeb76186dfec0ce86c82f14fe1d933c3996089"));

      Signature sigWrongV(Hex::toBytes("e7a9dc85504bf4f79732e55c25fded4dd5471dfc28a6e35463aab7e8dfb180b5414520f0e8f18ec174fc2e14ce4c98f12faf58344c00af87c50b7bc502ac6b5f04"));
      UPubKey keyWrongV = Secp256k1::recover(sigWrongV, msg);
      REQUIRE_THAT(keyWrongV.hex(), Equals("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
      // TODO: how can parse_compact() and recover() fail?
    }

    SECTION("Compose Signature") {
      uint256_t r("8234104122482341265491137074636836252947884782870784360943022469005013929455");
      uint256_t s("8234104122482341265491137074636836252947884782870784360943022469005013929455");
      uint8_t v(0x01);
      Signature sig = Secp256k1::makeSig(r, s, v);
      REQUIRE_THAT(sig.hex(), Equals("1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef01"));
    }

    SECTION("Verify Signature") {
      uint256_t r("8234104122482341265491137074636836252947884782870784360943022469005013929455");
      uint256_t s("8234104122482341265491137074636836252947884782870784360943022469005013929455");
      uint8_t v(0x01);

      uint256_t rErr("115792089237316195423570985008687907852837564279074904382605163141518161494338");
      uint256_t sErr("115792089237316195423570985008687907852837564279074904382605163141518161494338");
      uint8_t vErr(0x02);

      REQUIRE(Secp256k1::verifySig(r, s, v));
      REQUIRE(!Secp256k1::verifySig(rErr, s, v));
      REQUIRE(!Secp256k1::verifySig(r, sErr, v));
      REQUIRE(!Secp256k1::verifySig(r, s, vErr));
    }

    SECTION("Convert PrivKey to UPubKey") {
      PrivKey privKey1(Hex(std::string("d859e5181ed0c73b6e860e7e5d6cb6d579c01ea0467a2a524bf65cf9333deec4")).bytes());
      PrivKey privKey2(Hex(std::string("7ea62bea5fa637444955d7a7b89538e387f0e10d6318d83280672e34f882f306")).bytes());
      PrivKey privKey3(Hex(std::string("eaa0ab53b1048c607a0858b9cabdec4f3712337700c2899a00edec6c457892a6")).bytes());
      PrivKey privKey4(Hex(std::string("4f81c5528443a8a4049f99df9390ff42b0ba44ea64b05c1243580d539dfe1bbc")).bytes());
      PrivKey privKey5(Hex(std::string("fd99fef41924e5497bd65de069639ce9cbb072ea9977cbdd7ca0188171e3e93b")).bytes());

      UPubKey uPubKey1 = Secp256k1::toUPub(privKey1);
      UPubKey uPubKey2 = Secp256k1::toUPub(privKey2);
      UPubKey uPubKey3 = Secp256k1::toUPub(privKey3);
      UPubKey uPubKey4 = Secp256k1::toUPub(privKey4);
      UPubKey uPubKey5 = Secp256k1::toUPub(privKey5);

      REQUIRE_THAT(uPubKey1.hex(), Equals("0417f84019d59d96eeb28d0c49478b74135d8beca7bf0cdc89d1ce90a4cabadf8dddfa5af24b9399d5691a5967f3f353d5114bb3532a37993cdd42a0fc3cf2d46d"));
      REQUIRE_THAT(uPubKey2.hex(), Equals("0472b8f68e53e79b5d1da97c80ec5d96b775f3dadbcf9e82eb282ea03a9bc01ca33747cfe5f9ee02ec40dcf1a0f2a11a39bdc68dbf6ede50615902e289c90197f7"));
      REQUIRE_THAT(uPubKey3.hex(), Equals("042b353fe61c393061afc77c74074f1432b462615c5efa150353982d7277491e45bc0b783b1eff9c047909f0aea43ff8afd973c8cde1b8b03218b96e67753ba0f4"));
      REQUIRE_THAT(uPubKey4.hex(), Equals("04cdc0e4a1d3e802ca980bd9936b7926250eb6706488c85b297541867617829cf743064d0a7efb7f3501156cd3380dec5ac5df5a1a26da0445837d92fcc0bd4d68"));
      REQUIRE_THAT(uPubKey5.hex(), Equals("042fef7aff4cf83ba567dd16e05e1776505df5422bcca2b50544ba9e123cc2905a082f343946011b04da129db6e721775df87947729363a00ee1a5815b2b10fdcd"));
      // TODO: how can pubkey_create() fail?
    }

    SECTION("Convert PubKey to UPubKey") {
      PubKey cPubKey1(Hex(std::string("023598ec221cd9ee25ef316b58e706ec65df7623dbeb2e8e257a355e24adb3c328")).bytes());
      PubKey cPubKey2(Hex(std::string("034619ac0774250effece5abc5b9d85c8e88a967ae4d81e1b2c65252c2ab2d85c7")).bytes());
      PubKey cPubKey3(Hex(std::string("03d39792c2b11d8bba9f0cf3f484a8a661b1b69575b88714507d1bb2733fa4c84d")).bytes());
      PubKey cPubKey4(Hex(std::string("028846cf6b1c88dbdeb2fb3a32bd7adcfcfe451f8bac2e83e7a7958f3ee3664b2f")).bytes());
      PubKey cPubKey5(Hex(std::string("02c6fffc3fa01f5a0f81336b4e9d0d7fb8d5a83b1e163c3a555e538281946b2bed")).bytes());

      UPubKey uPubKey1 = Secp256k1::toUPub(cPubKey1);
      UPubKey uPubKey2 = Secp256k1::toUPub(cPubKey2);
      UPubKey uPubKey3 = Secp256k1::toUPub(cPubKey3);
      UPubKey uPubKey4 = Secp256k1::toUPub(cPubKey4);
      UPubKey uPubKey5 = Secp256k1::toUPub(cPubKey5);

      REQUIRE_THAT(uPubKey1.hex(), Equals("043598ec221cd9ee25ef316b58e706ec65df7623dbeb2e8e257a355e24adb3c32828185003cb4c7a9953016977d3c3230d7a17ccef6322ddedf2ebf456b8cfff74"));
      REQUIRE_THAT(uPubKey2.hex(), Equals("044619ac0774250effece5abc5b9d85c8e88a967ae4d81e1b2c65252c2ab2d85c7bb1c9c3de29c1f0743ea2d2f0f856da879e8aa932935363a40eb74b56dd32a03"));
      REQUIRE_THAT(uPubKey3.hex(), Equals("04d39792c2b11d8bba9f0cf3f484a8a661b1b69575b88714507d1bb2733fa4c84dbf85b6dcba1fd319a2276743507ab9e7ef1715c616abc2f35b2601b9c243dfdb"));
      REQUIRE_THAT(uPubKey4.hex(), Equals("048846cf6b1c88dbdeb2fb3a32bd7adcfcfe451f8bac2e83e7a7958f3ee3664b2f3947e9c6addf4b0325831fa9fca949bfdef7b475f1b684a8b06a38bc771491ce"));
      REQUIRE_THAT(uPubKey5.hex(), Equals("04c6fffc3fa01f5a0f81336b4e9d0d7fb8d5a83b1e163c3a555e538281946b2bedf480453cfea7bf4af8d98a3d5826b0b66d9e2b9b02c9f2b636631ebb9c27750c"));
      // TODO: how can pubkey_parse() fail?
    }
    SECTION("Convert PrivKey to PubKey") {
      PrivKey privKey1(Hex(std::string("bdc032aa80d06a37c52fc1423766cbcf7665b38c99dec2feecd29f1a9efbba62")).bytes());
      PrivKey privKey2(Hex(std::string("329bb613a122d78c419da15c8f31125b39a1083de7463bb3e46faf782edf3c54")).bytes());
      PrivKey privKey3(Hex(std::string("b3e481797e4768dc00bf8710528ce209ff95b5d074711c1a64db086cd29b95e9")).bytes());
      PrivKey privKey4(Hex(std::string("3f9026aa396487c9ddc22ee342580b5c5fd8842566377dc456145b0211672749")).bytes());
      PrivKey privKey5(Hex(std::string("595679c946f055c90d6640dbcd4988df08ea6d935cf508be2c0870d51ee48658")).bytes());

      PubKey cPubKey1 = Secp256k1::toPub(privKey1);
      PubKey cPubKey2 = Secp256k1::toPub(privKey2);
      PubKey cPubKey3 = Secp256k1::toPub(privKey3);
      PubKey cPubKey4 = Secp256k1::toPub(privKey4);
      PubKey cPubKey5 = Secp256k1::toPub(privKey5);

      REQUIRE_THAT(cPubKey1.hex(), Equals("03283fabe5d17d54f557552270155679378061e9f688d088f760754655b4b53f7c"));
      REQUIRE_THAT(cPubKey2.hex(), Equals("0274fcc0b61a26c84b923dc5cc84916f058610a0694fa1c2e96373d6a6597ed660"));
      REQUIRE_THAT(cPubKey3.hex(), Equals("03a786d1bab84d5847827f09400fbf83e1bbd7aed327a53b91eca3bb4e2d564257"));
      REQUIRE_THAT(cPubKey4.hex(), Equals("038a65b900d2a0756f427122b87c15fbe475856552109bfe2146fc759c5a48bb7e"));
      REQUIRE_THAT(cPubKey5.hex(), Equals("023eb250f22d6350ad336746ccd903a356bba1367307152433d54c23a008bf49c5"));
      // TODO: how can pubkey_create() fail?
    }

    SECTION("Convert UPubKey to Address") {
      UPubKey uPubKey1(Hex(std::string("04283fabe5d17d54f557552270155679378061e9f688d088f760754655b4b53f7c60bde73b95d4f868142e98a891d386a6fc59695d1472c22803c4f92cc86d39d1")).bytes());
      UPubKey uPubKey2(Hex(std::string("0474fcc0b61a26c84b923dc5cc84916f058610a0694fa1c2e96373d6a6597ed6601cf6e20dfa3bf2640d317e9fed647b1156752e818f57375edb8adab40cc067b4")).bytes());
      UPubKey uPubKey3(Hex(std::string("04a786d1bab84d5847827f09400fbf83e1bbd7aed327a53b91eca3bb4e2d56425768017480515348e26a198a5222f30d0b5cea4c0103429ad9e044b8d4de240d83")).bytes());
      UPubKey uPubKey4(Hex(std::string("048a65b900d2a0756f427122b87c15fbe475856552109bfe2146fc759c5a48bb7ecd8a79131cb2e4b5f3a01459bfb46b9edca230c4b7be6715fdbe9cd87ba3cfb5")).bytes());
      UPubKey uPubKey5(Hex(std::string("043eb250f22d6350ad336746ccd903a356bba1367307152433d54c23a008bf49c56791725889a4b83170b3e92f63c693a2534ac86d4a0a44beeb1de2fd0d448020")).bytes());

      Address a1 = Secp256k1::toAddress(uPubKey1);
      Address a2 = Secp256k1::toAddress(uPubKey2);
      Address a3 = Secp256k1::toAddress(uPubKey3);
      Address a4 = Secp256k1::toAddress(uPubKey4);
      Address a5 = Secp256k1::toAddress(uPubKey5);

      REQUIRE_THAT(a1.hex(), Equals("aab14e1f4704a9d3e0956ccbb2e1f284a83bd6f8"));
      REQUIRE_THAT(a2.hex(), Equals("3161be94eff2deacae7d7e5381349bc473d95809"));
      REQUIRE_THAT(a3.hex(), Equals("30cc20549ce7ab2cabd79968b44b9db6809bb3e5"));
      REQUIRE_THAT(a4.hex(), Equals("4b29825394d1948ef34152e20c4e05470ae85892"));
      REQUIRE_THAT(a5.hex(), Equals("739403b28c2f38b5b0b89557a3983d755f77c1a1"));
    }

    SECTION("Convert PubKey to Address") {
      PubKey cPubKey1(Hex(std::string("03283fabe5d17d54f557552270155679378061e9f688d088f760754655b4b53f7c")).bytes());
      PubKey cPubKey2(Hex(std::string("0274fcc0b61a26c84b923dc5cc84916f058610a0694fa1c2e96373d6a6597ed660")).bytes());
      PubKey cPubKey3(Hex(std::string("03a786d1bab84d5847827f09400fbf83e1bbd7aed327a53b91eca3bb4e2d564257")).bytes());
      PubKey cPubKey4(Hex(std::string("038a65b900d2a0756f427122b87c15fbe475856552109bfe2146fc759c5a48bb7e")).bytes());
      PubKey cPubKey5(Hex(std::string("023eb250f22d6350ad336746ccd903a356bba1367307152433d54c23a008bf49c5")).bytes());

      Address a1 = Secp256k1::toAddress(cPubKey1);
      Address a2 = Secp256k1::toAddress(cPubKey2);
      Address a3 = Secp256k1::toAddress(cPubKey3);
      Address a4 = Secp256k1::toAddress(cPubKey4);
      Address a5 = Secp256k1::toAddress(cPubKey5);

      REQUIRE_THAT(a1.hex(), Equals("aab14e1f4704a9d3e0956ccbb2e1f284a83bd6f8"));
      REQUIRE_THAT(a2.hex(), Equals("3161be94eff2deacae7d7e5381349bc473d95809"));
      REQUIRE_THAT(a3.hex(), Equals("30cc20549ce7ab2cabd79968b44b9db6809bb3e5"));
      REQUIRE_THAT(a4.hex(), Equals("4b29825394d1948ef34152e20c4e05470ae85892"));
      REQUIRE_THAT(a5.hex(), Equals("739403b28c2f38b5b0b89557a3983d755f77c1a1"));
    }

    SECTION("Sign Message and Recover Signature") {
      Hash msg = Utils::sha3(Utils::create_view_span(std::string("Hello World!")));
      PrivKey key(Hex::toBytes("21ce34f85520a26a29c8b4d94c883006a52f839fa61f13b86094b9fcdca558fa"));
      Signature sig = Secp256k1::sign(msg, key);
      REQUIRE_THAT(sig.hex(), Equals("de6235ae53213a745170457ff9c91292953ac73fc6da5cb7fd9ce48b440cd3b17f57934495e6ab9328018b05510bbfd18ce440dd37bc694c4ee17db846d4ad7301"));
      REQUIRE_THAT(Secp256k1::recover(sig, msg).hex(), Equals("044093c50188db83575dcbbf03cc5fde6feea015d6a1822b990cad3a17418889fd8d1cff667985c4a8547575e17527c98c41cc5fbcf680051f76a60ea6caa2c00b"));
      // TODO: how can sign_recoverable() fail?
      //std::cout << Secp256k1::recover(Signature(), msg).hex().get() << std::endl;
      //std::cout << Secp256k1::recover(sig, Hash()).hex().get() << std::endl;
      //REQUIRE_THAT(Secp256k1::recover().hex(), Equals(""));
    }

    SECTION("Sign Message and Verify Signature") {
      Hash msg = Utils::sha3(Utils::create_view_span(std::string("Hello World!")));
      UPubKey key(Hex::toBytes("04836c5d13e068e4d28d9cdfb22b2cf74628260edb4e6a54ec429b5c4f86728bc97f5ce677d27b0892579fe22ed7a0fec237388e232d3ec4848d4bc4b70681cb6e"));
      UPubKey fakeKey(Hex::toBytes("04836c5d13e068e4d28d9cdfb22b2cf74628260edb4e6a54ec429b5c4f86728bc97f5ce677d27b0892579fe22ed7a0fec237388e232d3ec4848d4bc4b70681cb6f"));
      Signature sig(Hex::toBytes("97026a63106bcc73fde07c53042df4940d571b510fa586d79baa8bd9252092681286b729bc30d7044e0e69e5f6246b5f9ff88c30c1cf0017e615867d565b977801"));
      REQUIRE(Secp256k1::verify(msg, key, sig));
      REQUIRE(!Secp256k1::verify(msg, fakeKey, sig));
    }
  }
}


