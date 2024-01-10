#include <cstring>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#define BUFFER_LEN 256
#define AVG_LEN 10


class DistanceDetect {
private:
	const int PT_CONST = 4;
	const float DELTA_TIME = 0.01;
	const float SPEEDOFSOUND = 343.0;

	char buffer[BUFFER_LEN];
	int serial_port;
	int count = 0;
	float avg_distance[AVG_LEN] = { 0 };

	int receive() {
		usleep(20 * 100); // wait some time for device send data

		tcflush(serial_port, TCIOFLUSH);
		usleep(DELTA_TIME * 1000000);
		int n = read(serial_port, buffer, sizeof(buffer));
		buffer[n] = '\0';
		return n;
	}

public:

	DistanceDetect(const char* serial_file_path) {
		serial_port = open(serial_file_path, O_RDWR | O_NOCTTY);
		if (serial_port < 0) {
			fprintf(stderr, "Error %i from open: %s\n", errno, strerror(errno));
			exit(1);
		}

		// Create new termios struct, we call it 'tty' for convention
		struct termios tty;

		// Read in existing settings, and handle any error
		if(tcgetattr(serial_port, &tty) != 0) {
			fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
			close(serial_port);
			exit(1);
		}

		// Clear parity bit, disabling parity (most common)
		tty.c_cflag &= ~PARENB;
		// Clear stop field, only one stop bit used in communication (most common)
		tty.c_cflag &= ~CSTOPB;
		// Clear all bits that set the data size
		tty.c_cflag &= ~CSIZE;
		// 8 bits per byte (most common)
		tty.c_cflag |= CS8;
		// Disable RTS/CTS hardware flow control (most common)
		tty.c_cflag &= ~CRTSCTS;
		// Turn on READ & ignore ctrl lines (CLOCAL = 1)
		tty.c_cflag |= CREAD | CLOCAL;

		tty.c_lflag &= ~ICANON;
		// Disable echo
		tty.c_lflag &= ~ECHO;
		// Disable erasure
		tty.c_lflag &= ~ECHOE;
		// Disable new-line echo
		tty.c_lflag &= ~ECHONL;
		// Disable interpretation of INTR, QUIT and SUSP
		tty.c_lflag &= ~ISIG;
		// Turn off s/w flow ctrl
		tty.c_iflag &= ~(IXON | IXOFF | IXANY);
		// Disable any special handling of received bytes
		tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

		// Prevent special interpretation of output bytes (e.g. newline chars)
		tty.c_oflag &= ~OPOST;
		// Prevent conversion of newline to carriage return/line feed
		tty.c_oflag &= ~ONLCR;
		// Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
		// tty.c_oflag &= ~OXTABS;
		// Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)
		// tty.c_oflag &= ~ONOEOT;

		// Wait for up to VTIME deciseconds.
		// Returning as soon as VMIN data is received.
		tty.c_cc[VTIME] = 1;
		tty.c_cc[VMIN] = 0;

		// Set in/out baud rate to be 9600
		cfsetispeed(&tty, B9600);
		cfsetospeed(&tty, B9600);

		// Save tty settings, also checking for error
		if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
			fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
			close(serial_port);
			exit(1);
		}
	}

	~DistanceDetect(){
		close(serial_port);
	}

	float get_ball_position(){
		if (receive() <= 0) {
			usleep(DELTA_TIME * 1000000);
			receive();
		}

		float distance = (std::stoi(buffer) * SPEEDOFSOUND) / (2.0f * 10000.0f) - 12;
		return distance;
	}

	float get_filted_position(){
		float sum = 0.0;
		float avg = 0.0;

		float distance = get_ball_position();
		if (count < 10){
			count ++;
		}else {
			count = 0;
		}
		avg_distance[count] = distance;

		for (int i = 0; i < AVG_LEN; ++i){
			sum += avg_distance[i];
		}

		avg = sum / AVG_LEN;
		return avg;
	}
};
