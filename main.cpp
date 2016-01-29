#include "mbed.h"
#include "USBMIDI.h"
#define MIN(a,b) ((a) < (b) ? (a) : (b))

USBMIDI midi;
Serial pc(USBTX, USBRX);
DigitalIn b1(p21);
DigitalIn b2(p22);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

int connected = 0;
int connecting = 0;

uint8_t message_header[] = { 0xF0, 0x00, 0x00, 0x66, 0x10 };
void transmitMCU(uint8_t * data, uint8_t len)
{
	unsigned char tmp[40];
	/*
	tmp[0] = 0xF0;
	tmp[1] = 0;
	tmp[2] = 0;
	tmp[3] = 0x66;
	tmp[4] = 0x10;
	tmp[5] = 1;
	tmp[6] = 'a';
	tmp[7] = 'a';
	tmp[8] = 'a';
	tmp[9] = 'a';
	tmp[10] = 'a';
	tmp[11] = 'a';
	tmp[12] = 'b';
	tmp[13] = 1;
	tmp[14] = 2;
	tmp[15] = 3;
	tmp[16] = 4;
	tmp[17] = 0xF7;
	midi.write(MIDIMessage::SysEx(tmp, 18));
	*/

	memcpy(tmp, message_header, sizeof(message_header));
	memcpy(&tmp[sizeof(message_header)], data, len);
	tmp[sizeof(message_header) + len] = 0xF7;
	midi.write(MIDIMessage::SysEx(tmp, len + sizeof(message_header) + 1));
}

uint8_t out_host_conn_qry[] = {0x01, 'T','e','s','t','e',' ',' ',0x01,0x02,0x03,0x04};
void send_host_connection_query(void) {
	transmitMCU(out_host_conn_qry, 12);
}

uint8_t out_host_conn_confirmation[8] = {0x03, 'T','e','s','t','e',' ',' '};
void send_host_connection_confirmation(void) {
	transmitMCU(out_host_conn_confirmation, 8);
}

uint8_t in_device_query[] = {0x00};
uint8_t in_host_connection_reply[] = {0x02, 'T','e','s','t','e',' ',' ',0x05,0x05,0x0B,0x6F};
uint8_t in_go_offline[] = {0x0F, 0x7F};
void decode_incomming_sys_ex(MIDIMessage msg) {
	switch (msg.length -1 -6) {
		case sizeof(in_device_query):
			send_host_connection_query();
			break;
		case sizeof(in_host_connection_reply):
			send_host_connection_confirmation();
			led4 = 1;
			break;
		default:
			break;
	}
}

// MIDI IN
void receive_message(MIDIMessage msg) {
	switch (msg.type()) {
		/*
		   case MIDIMessage::NoteOnType:
		   wait(0.1);
		   midi.write(MIDIMessage::NoteOn(msg.key()));
		   break;
		   case MIDIMessage::NoteOffType:
		   wait(0.1);
		   midi.write(MIDIMessage::NoteOff(msg.key()));
		   break;
		   case MIDIMessage::ProgramChangeType:
		   wait(0.1);
		   midi.write(MIDIMessage::ProgramChange(msg.program()));
		   break;
		 */
	case MIDIMessage::ChannelAftertouchType:
		{
		uint8_t val = msg.pressure();
		if ((val & 0xF0) == 0x00) { // Channel 1
			val &= 0x0F;
			if (val < 0x0E) { // Ignore set/clear overload
				led1 = val >= 2;
				led2 = val >= 5;
				led3 = val >= 8;
				led4 = val >= 11;
			}
		}
		}
		break;

	case MIDIMessage::SysExType:
		/*
		wait(0.1);
		unsigned char tmp[64];
		for(int i=0;i<msg.length-1;i++) {
			tmp[i]=msg.data[i+1];
		}
		midi.write(MIDIMessage::SysEx(tmp,msg.length-1));
		*/
		decode_incomming_sys_ex(msg);
		break;
	default:
		break;
	}
}

int main(void)
{
	wait(3);
#if 0
	wait(5);
	// MIDI OUT

	// set piano
	midi.write(MIDIMessage::ProgramChange(1));
	wait(0.1);

	// play A
	midi.write(MIDIMessage::NoteOn(21));
	wait(0.1);
	midi.write(MIDIMessage::NoteOff(21));
	wait(0.1);

	// GM reset
	unsigned char gm_reset[]={0xF0,0x7E,0x7F,0x09,0x01,0xF7};
	midi.write(MIDIMessage::SysEx(gm_reset,6));
	wait(0.1);

	// GM Master volume max
	unsigned char gm_master_vol_max[]={0xF0,0x7F,0x7F,0x04,0x01,0x7F,0x7F,0xF7};
	midi.write(MIDIMessage::SysEx(gm_master_vol_max,8));
	wait(0.1);

	// GS reset
	unsigned char gs_reset[]={0xF0,0x41,0x10,0x42,0x12,0x40,0x00,0x7F,0x00,0x41,0xF7};
	midi.write(MIDIMessage::SysEx(gs_reset,11));
	wait(0.1);

	// GS Master volume max
	unsigned char gs_master_vol_max[]={0xF0,0x41,0x10,0x42,0x12,0x40,0x00,0x04,0x7F,0x3D,0xF7};
	midi.write(MIDIMessage::SysEx(gs_master_vol_max,11));
	wait(0.1);
#endif

	midi.attach(receive_message);

	
#if 1
	int last1 = 0;
	int last2 = 0;
	while(1)
	{
		//led1 = b1;
		//led2 = b2;
		
		if (b1 != last1)
		{
			last1 = b1;
			if (last1)
			{
				//transmitMCU(out_host_conn_qry, 12);
				//midi.write(MIDIMessage::NoteOn(21));
				midi.write(MIDIMessage::NoteOn(0x10, 0x7F));
			}	
			else
			{
			//	midi.write(MIDIMessage::NoteOff(21));
				midi.write(MIDIMessage::NoteOn(0x10, 0x00));
			}
		}

		if (b2 != last2)
		{
			last2 = b2;
			if (last2)
				midi.write(MIDIMessage::NoteOn(0x11, 0x7F));
				//midi.write(MIDIMessage::NoteOn(22));
			else
				midi.write(MIDIMessage::NoteOn(0x11, 0x00));
				//midi.write(MIDIMessage::NoteOff(22));
		}

		wait(0.05);
	}
#endif
}
