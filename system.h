#define LEN 256

class System {
private:
	const float DELTA_TIME = 0.01;

	float positions[LEN];

public:
	System() {
		buffer = { 0 };
	}
	~System() {}

	void new_position(float pos) {
		for (int i = 1; i < LEN; ++i) {
			positions[i - 1] = positions[i];
		}
		positions[LEN] = pos;
	}

	float delta() {
		return (positions[LEN - 1] - positions[LEN - 2]) / DELTA_TIME;
	}

	float intergral(){
		float sum = 0;
		for (int i = 0; i < LEN; ++i){
			sum += positioins[i];  
		}
		return sum * DELTA_TIME; 
	}

	float pid(float P, float I, float D) {

	}
};
