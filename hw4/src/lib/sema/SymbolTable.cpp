#include "sema/SymbolTable.hpp"

#include <cstdio>

extern uint32_t opt_dump;

void SymbolManager::pushScope(SymbolTable *table) {
  tables.push(table);
  cur_table = table;
  cur_level++;
}

void SymbolManager::popScope() {
  tables.pop();
  cur_table = tables.empty() ? nullptr : tables.top();
  cur_level--;
}

static void dumpDemarcation(const char chr) {
  for (size_t i = 0; i < 110; ++i) {
    printf("%c", chr);
  }
  puts("");
}

void SymbolManager::dumpSymbolTable(SymbolTable *table) {
  if (opt_dump == 0)
    return;

  dumpDemarcation('=');
  printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
         "Attribute");
  dumpDemarcation('-');
  for (long unsigned int i = 0; i < table->entries.size(); i++) {
    printf("%-33s", table->entries[i].name.c_str());
    printf("%-11s", table->entries[i].getKindString().c_str());
    printf("%d%-10s", table->entries[i].level,
           table->entries[i].level ? "(local)" : "(global)");
    printf("%-17s", table->entries[i].type->getPTypeCString());
    printf("%-11s", table->entries[i].getAttrString().c_str());
    puts("");
  }
  dumpDemarcation('-');
}
