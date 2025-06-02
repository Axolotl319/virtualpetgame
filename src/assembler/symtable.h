typedef struct symbol_table{
	char **labels; //array of strings, size decided in first pass
	uint8_t *addresses; //associated address
	//label[i] has address addresses[i]
} symbol_table;
