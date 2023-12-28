#include <cstring>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#define BUFFER_LEN 256

enum class Mode {
	Position,
	Velocity,
	Torque,
};

class Motor {
private:
	const int VT_CONST = 32768;
	const float ADT_CONST = 4.096;
	const int PT_CONST = 4;

	char buffer[BUFFER_LEN];
	int serial_port;
	Mode mode;

	void send(const char* command) {
		if (write(serial_port, command, strlen(command)) < 0) {
			fprintf(stderr, "Error %i from Motor::send(%s): %s\n",
					errno, command, strerror(errno));
		}
	}

	int receive() {
		usleep(20 * 1000); // wait some time for device sand data
		int n = read(serial_port, buffer, sizeof(buffer));
		buffer[n] = '\0';
		return n;
	}

public:
	const float default_velocity = 10;
	const float default_acceleration = 5;

	const float pt_limit = 100000;
	const float v_limit = 50;
	const float a_limit = 50;

	Motor(const char* serial_file_path) {
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

		mode = Mode::Position;
		send("EIGN(2) EIGN(3) ZS MP\r");
		set_velocity(default_velocity);
		set_velocity(default_acceleration);
	}

	~Motor() {
		// stop the motor
		set_mode(Mode::Velocity);
		set_velocity(0);
		go();
		sleep(1);
		off();
		close(serial_port);
	}

	void go() { send("G\r"); }
	void stop() { send("X\r"); }
	void off() { send("OFF\r"); }
	void reset() { send("Z\r"); }

	void set_origin(int origin = 0) {
		sprintf(buffer, "O=%d\r", origin);
		send(buffer);
	}

	void set_velocity(float velocity) {
		sprintf(buffer, "VT=%d\r", (int)(velocity * VT_CONST));
		send(buffer);
	}

	void set_acceleration(float acceleration) {
		sprintf(buffer, "ADT=%d\r", (int)(acceleration * ADT_CONST));
		send(buffer);
	}

	void go_absolute_position(float absolute_position) {
		sprintf(buffer, "PT=%d G\r", (int)(absolute_position * PT_CONST));
		send(buffer);
	}

	void go_relative_position(float relative_position) {
		sprintf(buffer, "PRT=%d G\r", (int)(relative_position * PT_CONST));
		send(buffer);
	}

	float get_position() {
		send("RPA\r");
		receive();
		return (float)std::stoi(buffer) / PT_CONST;
	}

	float get_velocity() {
		send("RVC\r");
		receive();
		return (float)std::stoi(buffer) / VT_CONST;
	}

	float get_target_velocity() {
		send("RVT\r");
		receive();
		return (float)std::stoi(buffer) / VT_CONST;
	}

	float get_acceleration() {
		send("RAC\r");
		receive();
		return (float)std::stoi(buffer) / ADT_CONST;
	}

	float get_target_acceleration() {
		send("RAT\r");
		receive();
		return (float)std::stoi(buffer) / ADT_CONST;
	}

	void set_mode(Mode m) {
		mode = m;
		switch (mode) {
			case Mode::Position: send("MP\r"); break;
			case Mode::Velocity: send("MV\r"); break;
			case Mode::Torque:   send("MT\r"); break;
		}
	}

	Mode get_mode() {
		return mode;
	}
};
