/*
 * liza_game_mega8.c
 *
 * Created: 14.09.2017 22:27:16
 */ 
#define F_CPU 1000000UL // 1MHz

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

const unsigned char button1 = 0b1000;
const unsigned char button2 = 0b0100;
const unsigned char button3 = 0b0010;
const unsigned char button4 = 0b0001;

unsigned char button_count[3] = {0, 0, 0, 0};
const unsigned char button_count_qty = 20;

unsigned char button_read() {
	unsigned char button = 0;
	
	if ( !(PIND & (1 << PIND0)) ) {
		if ( button_count[0] < button_count_qty ) button_count[0]++;
	}
	else {
		button_count[0] = 0;
	}
	
	if ( !(PIND & (1 << PIND1)) ) {
		if ( button_count[1] < button_count_qty ) button_count[1]++;
	}
	else {
		button_count[1] = 0;
	}
	
	if ( !(PIND & (1 << PIND2)) ) {
		if ( button_count[2] < button_count_qty ) button_count[2]++;
	}
	else {
		button_count[2] = 0;
	}
	
	if ( !(PIND & (1 << PIND3)) ) {
		if ( button_count[3] < button_count_qty ) button_count[3]++;
	}
	else {
		button_count[3] = 0;
	}
	
	if ( button_count[0] >= button_count_qty ) button |= (1 << 3);
	if ( button_count[1] >= button_count_qty ) button |= (1 << 2);
	if ( button_count[2] >= button_count_qty ) button |= (1 << 1);
	if ( button_count[3] >= button_count_qty ) button |= (1 << 0);
	
	return button;
}

void led_on(unsigned char led) {
	if (led & (1 << 0)) PORTC |= (1 << PINC1);
	if (led & (1 << 1)) PORTC |= (1 << PINC2);
	if (led & (1 << 2)) PORTC |= (1 << PINC3);
	if (led & (1 << 3)) PORTC |= (1 << PINC4);
	if (led & (1 << 4)) PORTC |= (1 << PINC5);
}

void led_off(unsigned char led) {
	if (led & (1 << 0)) PORTC &= ~(1 << PINC1);
	if (led & (1 << 1)) PORTC &= ~(1 << PINC2);
	if (led & (1 << 2)) PORTC &= ~(1 << PINC3);
	if (led & (1 << 3)) PORTC &= ~(1 << PINC4);
	if (led & (1 << 4)) PORTC &= ~(1 << PINC5);
}

void delay(unsigned int n) {
	while (n--) {
		_delay_ms(1);
	}
}

void tone(unsigned char freq, unsigned int delay_ms) {
	//OCR1A = freq;
	OCR2 = freq;
	delay(delay_ms);
	//OCR1A = 0;
	OCR2 = 0;
}

void blink_tone(unsigned char led, unsigned char count,
unsigned int delay_on, unsigned int delay_off,
unsigned char freq_on, unsigned int delay_ms_on,
unsigned char freq_off, unsigned int delay_ms_off) {
	while (count--) {
		led_on(led);
		tone(freq_on, delay_ms_on);
		delay(delay_on);
		led_off(led);
		tone(freq_off, delay_ms_off);
		delay(delay_off);
	}
}

void blink(unsigned char led, unsigned char count, unsigned int delay_on, unsigned int delay_off) {
	blink_tone(led, count, delay_on, delay_off, 0, 0, 0, 0);
}

unsigned char btn = 0;
unsigned char game_led = 0;
unsigned char game_state = 0;

void game1_start() {
	if ( (game_state == 0) && (button_read() == 0) ) {
		led_off(0b11111);
		led_on (0b10001);
		tone(1, 20);
		delay(500);
		game_state++;
	}
	
	if (game_state == 1) {
		if (button_read() != 0) game_state = 0;
		led_off(0b11111);
		led_on (0b01010);
		tone(2, 20);
		delay(500);
		game_state++;
	}
	
	if (game_state == 2) {
		if (button_read() != 0) game_state = 0;
		led_off(0b11111);
		led_on (0b00100);
		tone(3, 10);
		btn = 0;
		game_state++;
	}
	
	if (game_state == 3) {
		if (btn == 0) {
			btn = button_read();
			game_led = 0;
			} else {
			game_state++;
		}
	}
	
	if (game_state == 4) {
		led_off(0b11111);
		delay(200);
		
		if (btn == button1) game_led = 0b10000;
		if (btn == button2) game_led = 0b01000;
		if (btn == button3) game_led = 0b00010;
		if (btn == button4) game_led = 0b00001;
		
		
		//blink(led, 3, 100, 50);
		blink_tone(game_led, 3, 100, 50, 3, 15, 3, 15);
		
		led_off(0b11111);
		delay(1000);
		game_state = 0;
	}
}


void game2_init() {
	game_state = 0;
	game_led = 0b10000;
	btn = 0;
}

void game2_start() {
	
	if ((game_state == 0) && (button_read() == 0)) {
		game2_init();
		led_off(0b11111);
		
		OCR1A = ( (8 + (rand() % 10)) << 10);
		//OCR1A = ( (7 + (9)) << 10);;

		led_on(game_led);
		game_state++;
	}
	
	if (game_state == 1) {
		
		if (TIFR & (1 << OCF1A)) {  // count reached?
			TIFR |= (1 << OCF1A);   // clear flag
			led_off(game_led);
			game_led = game_led >> 1;
			if (game_led == 0) game_led = 0b10000;
			led_on(game_led);
			
			if (game_led != 0b00100) tone(1, 15); else tone(3, 20);
		}

		if ( button_read() != 0 ) {
			game_state++;
		}
		
	}
	
	if (game_state == 2) {
		delay(800);
		led_off(0b11111);

		if (game_led == 0b00100) {
			blink_tone(0b11011, 3, 100, 50, 1, 20, 1, 20);
			} else {
			blink_tone(0b00100, 3, 100, 50, 3, 15, 3, 15);
		}
		delay(500);
		game_state = 0;
	}
	
}

unsigned char game3_level = 1;
unsigned char game3_rnd_arr[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char game3_usr_arr[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char game3_btn_count = 0;

void game3_init() {
	game_state = 0;
	game3_level = 1;
	btn = 0;
}

void game3_start() {
	unsigned char i = 0;
	
	if ( game_state == 0 ) {
		game3_init();
		game_state++;
	}
	
	if ((game_state == 1) && (button_read() == 0)) {
		blink_tone(0b10001, 1, 100, 100, 1, 10, 1, 0);
		blink_tone(0b01010, 1, 100, 100, 1, 10, 1, 0);
		blink_tone(0b00100, 1, 100, 100, 1, 10, 1, 0);
		
		for (i = 0; i < game3_level; i++) {
			game3_rnd_arr[i] = rand() % 4;
			if ( game3_rnd_arr[i] >= 2 ) game3_rnd_arr[i]++;
		}
		delay(500);
		game_state++;
	}
	
	if ( game_state == 2 ) {
		for (i = 0; i < game3_level; i++) {
			led_on(1 << game3_rnd_arr[i]);
			tone(1, 20);
			delay(500);
			led_off(0b11111);
			delay(500);
		}
		btn = 0;
		game3_btn_count = 0;
		game_state++;
	}
	
	if ( game_state == 3 ) {
		btn = button_read();
		if (btn != 0) {
			led_off(0b11111);
			delay(100);

			if (btn == button1) {
				game3_usr_arr[game3_btn_count] = 4;
				game_led = 0b10000;
			}
			if (btn == button2) {
				game3_usr_arr[game3_btn_count] = 3;
				game_led = 0b01000;
			}
			if (btn == button3) {
				game3_usr_arr[game3_btn_count] = 1;
				game_led = 0b00010;
			}
			if (btn == button4) {
				game3_usr_arr[game3_btn_count] = 0;
				game_led = 0b00001;
			}
			
			led_on(game_led);
			tone(1, 20);
			tone(2, 15);
			tone(1, 20);
			delay(200);
			
			
			btn = 0;
			delay(500);
			game3_btn_count++;
		}
		
		if (game3_btn_count >= game3_level) game_state++;
	}
	
	if ( game_state == 4 ) {
		led_off(0b11111);
		
		tone(3, 10);
		delay(200);
		tone(3, 10);
		delay(200);
		tone(3, 10);
		delay(400);
		
		for (i = 0; i < game3_level; i++) {
			led_on(1 << game3_usr_arr[i]);
			tone(2, 20);
			delay(250);
			led_off(0b11111);
			delay(250);
		}
		game_state++;
	}
	
	if ( game_state == 5 ) {
		game_led = 0;
		for (i = 0; i < game3_level; i++) {
			if ( game3_rnd_arr[i] != game3_usr_arr[i] ) {
				game_led = 0b00100;
				break;
			}
		}
		
		delay(300);
		
		if (game_led == 0) {
			blink_tone(0b11011, 3, 100, 50, 1, 20, 1, 20);
			game3_level++;
			} else {
			blink_tone(0b00100, 3, 100, 50, 3, 15, 3, 15);
			game3_level--;
		}
		
		if ( game3_level < 1  ) game3_level = 1;
		if ( game3_level > 20 ) game3_level = 20;
		
		game_state = 1;
		delay(1000);
		
	}
}

unsigned char menu_state = 0;
void menu() {
	if (menu_state == 0) led_on(0b11010);
	
	if (menu_state == 1) game1_start();
	if (menu_state == 2) game2_start();
	if (menu_state == 3) game3_start();
	
	btn = button_read();
	if (btn != 0) {
		if (menu_state == 0) {
			if (btn == button1) menu_state = 1;
			if (btn == button2) menu_state = 2;
			if (btn == button3) menu_state = 3;
		}
	}

}

void interrupt() {
	if ( menu_state == 255) {
		sleep_disable();
		led_on(0b11111);
		delay(1000);
		led_off(0b11111);
		delay(200);
		menu_state = 0;
	}
		
	if ( !(PIND & ((1 << PIND2)| (1 << PIND3)) ) ) {
		
		if ( menu_state == 0) {
			menu_state = 255;
			delay(500);
			led_off(0b11111);
			delay(500);
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_enable();
			sei();
			sleep_cpu();
		}
		
		if ( (menu_state != 0) && (menu_state != 255) ) {
			menu_state = 0;
			game_state = 0;
			led_on(0b11111);
			delay(1000);
			led_off(0b11111);
			delay(300);
		}
	}
}

int main(void)
{
	DDRC |=  ( (1 << PINC5) | (1 << PINC4) | (1 << PINC3) | (1 << PINC2) | (1 << PINC1) );  // output (led)
	DDRB |=  ( (1 << PINB3) ); // output buzzer
	
	DDRD &= ~( (1 << PIND0) | (1 << PIND1) | (1 << PIND2) | (1 << PIND3) );  // input
	PORTD |= ( (1 << PIND0) | (1 << PIND1) | (1 << PIND2) | (1 << PIND3) );  // pull-up
	
	// buzzer PWM 
	OCR2 = 0; // set PWM for 0% duty cycle
	TCCR2 |= (1 << COM21);
	TCCR2 |= (0 << WGM21) | (1 << WGM20); // phase correct PWM mode
	TCCR2 |= (1 << CS21); // clock source = CLK/8, start PWM
	
	// timer for game2
	OCR1A = 0; // initial PWM pulse width
	TCCR1A = 0; // Normal port operation
	TCCR1B = (1 << WGM12) | (1 << CS11); // CTC mode, clk src = CLK/8, start timer
	
	GICR  |=  ( (1 << INT0) | (0 << INT1) );
	MCUCR |=  ( (0 << ISC11) | (0 << ISC11) | (0 << ISC01) | (0 << ISC00) | (1 << SM1 ) );
	
	sei();
		
    while (1) 
    {
		/*
		if (button_read() == button1) led_on(0b10000); else led_off(0b11111);
		if (button_read() == button2) led_on(0b01000); else led_off(0b11111);
		if (button_read() == button3) led_on(0b00010); else led_off(0b11111);
		if (button_read() == button4) led_on(0b00001); else led_off(0b11111);
		
		if (button_read() == (button1 | button4)) led_on(0b00100); else led_off(0b11111);
		*/
		
		/*
		if (button_read() == button1) tone(1, 30);
		if (button_read() == button2) tone(2, 30);
		if (button_read() == button3) tone(3, 30);
		if (button_read() == button4) tone(4, 30);
		*/
		
		menu();
    }
}

ISR (INT0_vect) {
	interrupt();
}
