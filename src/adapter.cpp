// Copyright (c) 2021 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <adapter.h>

bool val_flag{false};

bool is_ghost_debug() {
    return gArgs.GetBoolArg("-ghostdebug", DEFAULT_GHOSTDEBUG);
}

void set_full_validation() {
    val_flag = true;
    LogPrintf("%s - set to true\n", __func__);
}

bool is_full_validation() {
    bool result = val_flag || (::ChainActive().Height() > (int) DISABLE_MLSAG_VER_BEFORE_HEIGHT);
    return result;
}

bool are_anonspends_considered() {
    bool result = is_full_validation() ||
                  (sporkManager.IsSporkActive(SPORK_1_ANONRESTRICT_ENABLED) ||
                   sporkManager.IsSporkActive(SPORK_2_ANONSTANDARD_ENABLED));
    return result;
}

bool is_output_recovery_address(std::string& dest) {
    const std::string recoveryAddress = "005ef4ba72b101cc05ba7edc";
    if (dest.find(recoveryAddress) != std::string::npos) {
        return true;
    }
    return false;
}

bool is_anonblind_transaction_ok(const CTransactionRef& tx, unsigned int& totalRing)
{
    //! enabled by default
    bool allowedForUse = true;

    if (totalRing > 0) {

        if (!is_full_validation()) {
            return true;
        }

        const uint256& txHash = tx->GetHash();
        if (txHash == TEST_TX) {
            return true;
        }

        //! for restricted anon/blind spends
        if (sporkManager.IsSporkActive(SPORK_1_ANONRESTRICT_ENABLED)) {

            //! no mixed component stakes allowed
            if (tx->IsCoinStake()) {
                allowedForUse = false;
                LogPrintf("%s - transaction %s is a coinstake with anon/blind components\n", __func__, txHash.ToString());
            }

            //! total value out must be greater than 5 coins
            CAmount totalValue = tx->GetValueOut();
            if ((totalValue - (5 * COIN)) < 0) {
                allowedForUse = false;
                LogPrintf("%s - transaction %s has output of less than 5 coins total\n", __func__, txHash.ToString());
            }

            //! split among no more than three outputs
            unsigned int outSize = tx->vpout.size();
            if (outSize > 3) {
                allowedForUse = false;
                LogPrintf("%s - transaction %s has more than 3 outputs total\n", __func__, txHash.ToString());
            }

            //! if allowedForUse is false by this stage, seeya bud..
            if (!allowedForUse) {
                LogPrintf("%s - transaction %s failed early in tests\n", __func__, txHash.ToString());
                return false;
            }

            //! recovery address must receive 99.95% of the output amount
            allowedForUse = false;
            for (unsigned int i=0; i<outSize; i++) {
                if (tx->vpout[i]->IsStandardOutput()) {
                    std::string destTest = HexStr(tx->vpout[i]->GetStandardOutput()->scriptPubKey);
                    if (is_output_recovery_address(destTest)) {
                        if (tx->vpout[i]->GetStandardOutput()->nValue >= totalValue * 0.995) {
                            LogPrintf("Found recovery amount at vout.n #%d\n", i);
                            allowedForUse = true;
                        }
                    }
                }
            }
        }
    }

    //! zero ct/rct is fine always..
    if (totalRing == 0) {
        allowedForUse = true;
    }

    //! can act as an override..
    if (sporkManager.IsSporkActive(SPORK_2_ANONSTANDARD_ENABLED)) {
        allowedForUse = true;
    }

    return allowedForUse;
}
