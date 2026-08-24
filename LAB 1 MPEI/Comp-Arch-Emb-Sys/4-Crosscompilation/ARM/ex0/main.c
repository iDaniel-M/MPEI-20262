volatile unsigned int *const USART1_PTR = (unsigned int *)0x40011004;

void delay(int time){
	for(volatile int i=0; i<time; i++);
}

int main(void) {

	char s[14] = "Hello world!\n";
	int i;

	while(1){
		for(i=0; i<14; i++){
			*USART1_PTR = (unsigned int)s[i];
		}

		delay(10000000);
	}
}
