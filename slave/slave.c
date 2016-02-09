#include <avr/io.h>
#include <util/delay.h>

#include "SedProt.h"

int main(){

	sedProtSlaveInit();

	while (1) {

			_delay_ms(1000);
	
	};
}
