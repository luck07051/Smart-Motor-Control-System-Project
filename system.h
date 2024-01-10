#define LEN 256

class ControlSystem {
private:
	const float DELTA_TIME = 0.01;
	float positions[LEN] = { 0 };

	float differential() {
		return (positions[LEN - 1] - positions[LEN - 2]) / DELTA_TIME;
	}

	float intergral(){
		float sum = 0;
		for (int i = 0; i < LEN; ++i){
			sum += positions[i];
		}
		return sum * DELTA_TIME;
	}

public:
	int window = 10;

	ControlSystem() {}
	~ControlSystem() {}

	void new_position(float pos) {
		for (int i = 1; i < LEN; ++i) {
			positions[i - 1] = positions[i];
		}
		positions[LEN - 1] = pos;
	}

	float pid(float P, float I, float D) {
		return P * positions[LEN - 1] + I * intergral() + D * differential();
	}
};
