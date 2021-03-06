// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arith_uint256.h>
#include <primitives/block.h>
#include <streams.h>

#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>

extern "C" void cn_slow_hash(const void *data, size_t length, char *hash, int variant, int prehashed, uint64_t height);
extern "C" void cn_fast_hash(const void *data, size_t length, char *hash);

uint256 CBlockHeader::GetOriginalBlockHash() const
{
    CHashWriter hashWriter(SER_GETHASH, PROTOCOL_VERSION);
    hashWriter.write(BEGIN(nVersion), 80);
    return hashWriter.GetHash();
}

// prev_id of CN header is used to store the kevacoin block hash.
// The value of prev_id and block hash must be the same to prove
// that PoW has been properly done.
bool CBlockHeader::isCNConsistent() const
{
    return (GetOriginalBlockHash() == cnHeader.prev_id);
}

uint256 CBlockHeader::GetHash() const
{
    uint256 thash;
    if (!isCNConsistent()) {
        memset(thash.begin(), 0xff, thash.size());
        return thash;
    }
    cryptonote::blobdata blob = cryptonote::t_serializable_object_to_blob(cnHeader);
    cn_fast_hash(blob.data(), blob.size(), BEGIN(thash));
    return thash;
}

uint256 CBlockHeader::GetPoWHash() const
{
    uint256 thash;
    if (!isCNConsistent()) {
        memset(thash.begin(), 0xff, thash.size());
        return thash;
    }
    cryptonote::blobdata blob = cryptonote::t_serializable_object_to_blob(cnHeader);
    cn_slow_hash(blob.data(), blob.size(), BEGIN(thash), cnHeader.major_version - 6, 0, nNonce);
    return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetOriginalBlockHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
