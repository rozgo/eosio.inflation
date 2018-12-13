# EOSIO Contract for inflating an eosio.token


Setup accounts
```
cleos create account eosio fud.token <pub_key> <pub_key> 
cleos create account eosio fud.bank <pub_key> <pub_key>
```

Setup eosio.token
```
cleos set contract fud.token ~/contracts/eosio.contracts/eosio.token -p fud.token
cleos push action fud.token create '[ "fud.bank", "1000000.0000 FUD"]' -p fud.token@active
cleos push action fud.token issue '[ "fud.bank", "1000.0000 FUD", "token supply increase by bank deposit" ]' -p fud.bank@active
```

Setup inflation contract
```
eosio-cpp -o inflation.wasm inflation.cpp --abigen
cleos set contract fud.bank ~/contracts/fud.bank/inflation -p fud.bank@active
cleos set account permission fud.bank active --add-code
cleos push action fud.bank init "[]" -p fud.bank@active
```

Inspect and inflate
```
cleos get table fud.bank fud.bank issuance
cleos get table fud.token FUD stat
cleos push action fud.bank inflate '[]' -p fud.bank@active
```
