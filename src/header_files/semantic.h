#pragma once

symtable_record_local_t* check_stack(Stack* local_tables, char* identifier);

error_code_t semantic(TreeNode *node);


