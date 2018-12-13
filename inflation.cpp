#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;

class [[eosio::contract]] inflation : public contract
{
    const name TOKEN_ACCOUNT = "fud.token"_n;
    const symbol TOKEN_SYMBOL = symbol("FUD", 4);
    const name BANK_ACCOUNT = "fud.bank"_n;
    const double SECS_IN_YEAR = 60 * 60 * 24 * 365;
    const double SECS_IN_HOUR = 60 * 60;
    const double PERCENT_INFLATION_PER_ANUM = 5;
    const double PERCENT_INFLATION_PER_SEC = PERCENT_INFLATION_PER_ANUM / SECS_IN_YEAR;

  public:
    using contract::contract;

    inflation(name receiver, name code, datastream<const char *> ds) :
    contract(receiver, code, ds),
    issuances(receiver, code.value) {}

    [[eosio::action]] void init() {
        require_auth(BANK_ACCOUNT);
        auto issuances_itr = issuances.begin();
        eosio_assert(issuances_itr == issuances.end(), "Contract is init");
        issuances.emplace(_self, [&](auto& g){
            g.last = now();
            g.amount = 0;
        });
    }

    [[eosio::action]] void inflate() {
        require_auth(BANK_ACCOUNT);
        auto issuance = issuances.begin();
        uint32_t current = now();
        uint32_t lapsed = current - issuance->last;
        stats stat(TOKEN_ACCOUNT, TOKEN_SYMBOL.code().raw());
        auto token = *stat.begin();
        asset inflation(token.supply.amount * (PERCENT_INFLATION_PER_SEC / 100.0) * lapsed, TOKEN_SYMBOL);
        // eosio_assert(lapsed > SECS_IN_HOUR, "Inflate interval too short");
        eosio_assert(inflation.amount > 0, "Inflation too low");
        action(
            permission_level{ BANK_ACCOUNT, "active"_n },
            TOKEN_ACCOUNT,
            "issue"_n,
            std::make_tuple(
                BANK_ACCOUNT, 
                inflation,
                std::string("token supply increase by inflation")
            )
		).send();
        issuances.modify(issuance, BANK_ACCOUNT, [&](auto &g) {
            g.last = current;
            g.amount = inflation.amount;
        });
    }

private:

    struct [[eosio::table]] issuance {
        uint32_t last;
        uint32_t amount;
        uint64_t primary_key() const { return 0; }
    };
    typedef eosio::multi_index<"issuance"_n, issuance> issuance_index;
    issuance_index issuances;

    struct [[eosio::table]] currency_stats {
        asset supply;
        asset max_supply;
        name issuer;
        uint64_t primary_key()const { return supply.symbol.code().raw(); }
    };
    typedef eosio::multi_index< "stat"_n, currency_stats > stats;
};

EOSIO_DISPATCH(inflation, (init)(inflate))
