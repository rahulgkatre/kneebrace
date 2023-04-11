#define WINDOW_SIZE 10

class WindowFilterXYZ {
   private:
    int num_readings;
    xyz_t sum;
    xyz_t prev_vals[WINDOW_SIZE];

   public:
    void update(xyz_t* reading) {
        if (num_readings < WINDOW_SIZE) {
            prev_vals[num_readings] = *reading;
        } else {
            sum -= prev_vals[num_readings % WINDOW_SIZE];
            prev_vals[num_readings % WINDOW_SIZE] = *reading;
        }
        sum += *reading;
        num_readings += 1;
        reading->x = sum.x / std::min(WINDOW_SIZE, num_readings);
        reading->y = sum.y / std::min(WINDOW_SIZE, num_readings);
        reading->z = sum.z / std::min(WINDOW_SIZE, num_readings);
    }
};
