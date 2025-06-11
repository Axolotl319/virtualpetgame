typedef enum decode_flag {
	DCD_FAIL = -1, //fail
	DCD_SUCCESS,   //success
	DCD_HLT,       //halt instruction reached
	DCD_BRANCH     //branch instruction detected
} decode_flag;
