#pragma once

enum error_codes {
    ERR_NONE = 0,

    ERR_LEX_ANALYSIS = 1, 

    ERR_SYNTAX_ANALYSIS = 2, // chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu, chy- bějící hlavička, atp.)

    ERR_SEMANTIC_DEFINITION = 3,  // sémantická chyba v programu – nedefinovaná funkce, redefinice proměnné.

    ERR_SEMANTIC_FUNC = 4, // sémantická chyba v programu – špatný počet/typ parametrů u volání funkce či špatný typ návratové hodnoty z funkce

    ERR_SEMANTIC_NOT_DEFINED = 5, // sémantická chyba v programu – použití nedefinované nebo neinicializované proměnné

    ERR_SEMANTIC_RETURN = 6, // sémantická chyba v programu – chybějící/přebývající výraz v příkazu návratu z funkce

    ERR_SEMANTIC_TYPE_COMPATIBILITY = 7, //sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech

    ERR_SEMANTIC_TYPE = 8, // sémantická chyba odvození typu – typ proměnné nebo parametru není uveden a nelze odvodit od použitého výrazu.

    ERR_SEMANTIC_OTHERS = 9, // ostatní sémantické chyby.

    ERR_INTERNAL = 99, // interní chyba prekládače tj. neovlivněná vstupním programem (např. chyba alokace paměti, atd.).

};

typedef enum error_codes error_code_t;