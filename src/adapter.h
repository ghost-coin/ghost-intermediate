// Copyright (c) 2021 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADAPTER_H
#define ADAPTER_H

#include <chainparams.h>
#include <consensus/params.h>
#include <key_io.h>
#include <primitives/transaction.h>
#include <spork.h>
#include <util/system.h>
#include <validation.h>

const int LAST_ANONINDEX = 2379;
const uint256 TEST_TX = uint256S("c22280de808fdc24e1831a0daa91f34d01b93186d8f02e780788ed9f2c93aa24");

bool is_ghost_debug();
void set_full_validation();
bool is_full_validation();
bool are_anonspends_considered();
bool is_output_recovery_address(CPubKey& pubkey);
bool is_anonblind_transaction_ok(const CTransactionRef& tx, unsigned int& totalRing);

#endif // ADAPTER_H
