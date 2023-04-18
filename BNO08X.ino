#include "sh2_SensorValue.h"
#include <cmath>
#include <string>

// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9

// Pins for I2C mode - RP2040
#define BNO08X_SDA 2
#define BNO08X_SCL 3
// Pins for I2C mode - ESP32
// #define BNO08X_SDA 21
// #define BNO08X_SCL 22

// Print Out
#define PRINT true

// No Reset
#define BNO08X_RESET -1
  
// Analysis
#define WINDOW_SIZE 25 // For 100 Hz, assuming avg height and therefore gait of 2 hz, window size 12 would give us 1/4 of a gait.
#define INFLUENCE 0.06 // Influence of new values on stddev
#define THRESHOLD 3.2 // Threshold for peak detection
#define PEAK_DEADZONE_Y 1.5 // Peak Deadzone for gyro
#define DELTA_ALIVEZONE 0.6 // Peak Deadzone for gyro
#define PEAK_SLEEP 20 // Peak Deadzone for gyro


/* ======================================================================================================================== */
/* ======================================================================================================================== */
/* ======================================================================================================================== */


// Datatype for storing accelerometer (linear) and gyro information
struct xyz_t {
  float x;
  float y;
  float z;
  xyz_t() : x(0), y(0), z(0) {}
  xyz_t(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}
  xyz_t(const xyz_t& other) : x(other.x), y(other.y), z(other.z) {}
  void set(float nx, float ny, float nz) {
    x = nx;
    y = ny;
    z = nz;
    return;
  }
  xyz_t& operator=(const xyz_t& val) {
    x = val.x;
    y = val.y;
    z = val.z;
    return *this;
  }
  xyz_t operator+(const xyz_t& val) const {
    return xyz_t(x + val.x, y + val.y, z + val.z);
  }
  xyz_t operator-(const xyz_t& val) const {
    return xyz_t(x - val.x, y - val.y, z - val.z);
  }
  xyz_t& operator+=(const xyz_t& val) {
    x += val.x;
    y += val.y;
    z += val.z;
    return *this;
  }
  xyz_t& operator-=(const xyz_t& val) {
    x -= val.x;
    y -= val.y;
    z -= val.z;
    return *this;
  }
};

// Orientation in the world frame
struct euler_t {
  float yaw;
  float pitch;
  float roll;
  void set(float ny, float np, float nr) {
    yaw = ny;
    pitch = np;
    roll = nr;
    return;
  }
};

// Raw output from the sensor
struct quaternion_t {
  float real;
  float i;
  float j;
  float k;
  void set(float nr, float ni, float nj, float nk) {
    real = nr;
    i = ni;
    j = nj;
    k = nk;
    return;
  }
};

// Step counter from initialization
struct steps_t {
  int steps;
  float latency;
  void set(float ns, float nl) {
    steps = ns;
    latency = nl;
    return;
  }
};

float magnitude(xyz_t* input, bool sqrt = false) {
  float sum = input->x * input->x + input->y * input->y + input->z * input->z;
  if (sqrt) {
    sum = std::sqrt(sum);
  }
  return sum;
}

void quaternionToEuler(float qr, float qi, float qj, float qk, euler_t* ypr, bool degrees = false) {

    float sqr = sq(qr);
    float sqi = sq(qi);
    float sqj = sq(qj);
    float sqk = sq(qk);

    ypr->yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr));
    ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
    ypr->roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));

    if (degrees) {
      ypr->yaw *= RAD_TO_DEG;
      ypr->pitch *= RAD_TO_DEG;
      ypr->roll *= RAD_TO_DEG;
    }
    return;
}

void quaternionToEulerRV(sh2_RotationVectorWAcc_t* rotational_vector, euler_t* ypr, bool degrees = false) {
    quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
    return;
}

void quaternionToEulerGI(sh2_GyroIntegratedRV_t* rotational_vector, euler_t* ypr, bool degrees = false) {
    quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
    return;
}


/* ======================================================================================================================== */
/* ======================================================================================================================== */
/* ======================================================================================================================== */

// TODO function defines whether peak is peak depending on certain criteria...
bool natural_gait(float y_val, float delta_y) {
  return std::abs(y_val) > PEAK_DEADZONE_Y && std::abs(delta_y) < DELTA_ALIVEZONE;
}

class window_filter_xyz {
private:
  int num_readings;
  xyz_t sum;
  xyz_t avg;
  xyz_t prev_avg;
  xyz_t prev_vals[WINDOW_SIZE];

  xyz_t prev_delta;
  xyz_t delta;
  bool hit_peak_y = 0;
  int last_peak = 0;
  int last_pos = 0;
  int last_neg = 0;
  int second_last_neg = 0;
  float stance_percent = 0;
  float second_last_neg_value = 0;
  float last_neg_value = 0;
  float neg_peak_ratio = 0;

  xyz_t prev_var;
  xyz_t var; 

  int steps = 0;
public:
  window_filter_xyz() : num_readings(0), sum(xyz_t()), avg(xyz_t()), prev_avg(xyz_t()), prev_var(xyz_t()), var(xyz_t()) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
      prev_vals[i] = xyz_t();
    }
  }
  // Applies moving average and modifies existing reading's values.
  void update(xyz_t* reading) {

    // For std
    xyz_t prev_reading = xyz_t();
    xyz_t prev_avg = xyz_t(avg);

    // Square reading to make peaks more visible?
    reading->x = reading->x * 4;
    reading->y = reading->y * 4;
    reading->z = reading->z * 4;

    // Calculate rolling sum
    if (num_readings < WINDOW_SIZE) {
      prev_vals[num_readings] = *reading;
    } else {
      prev_reading = xyz_t(prev_vals[num_readings % WINDOW_SIZE]);
      sum -= prev_reading;
      prev_vals[num_readings % WINDOW_SIZE] = *reading;
    }
    sum += *reading;
    num_readings += 1;

    // Find averages
    prev_avg = avg;
    avg.x = sum.x / std::min(WINDOW_SIZE, num_readings);
    avg.y = sum.y / std::min(WINDOW_SIZE, num_readings);
    avg.z = sum.z / std::min(WINDOW_SIZE, num_readings);
    prev_delta = delta;
    delta = avg - prev_avg;

    if (num_readings - last_peak > PEAK_SLEEP && (delta.y > 0 && prev_delta.y < 0 && avg.y < 0 || delta.y < 0 && prev_delta.y > 0 && avg.y > 0)) {
      hit_peak_y = 1;
      last_peak = num_readings;
    } else if (num_readings - last_peak > 4) {
      hit_peak_y = 0;
    }

    // Variance
    prev_var = var;
    var += xyz_t((INFLUENCE * (reading->x - prev_reading.x) * (reading->x - avg.x + prev_reading.x - prev_avg.x)) / (WINDOW_SIZE-1), 
            + (INFLUENCE * (reading->y - prev_reading.y) * (reading->y - avg.y + prev_reading.y - prev_avg.y)) / (WINDOW_SIZE-1), 
            + (INFLUENCE * (reading->z - prev_reading.z) * (reading->z - avg.z + prev_reading.z - prev_avg.z)) / (WINDOW_SIZE-1));

    return;
  }
  xyz_t* get_avg() {
    return &avg;
  }
  float get_std(bool avg=true) {
    float sum = var.x + var.y + var.z;
    if (sum < 0) {
      return 0;
    }
    return avg ? std::sqrt(sum / 3) : std::sqrt(sum);
  }
  float get_prev_std_y() {
    if (prev_var.y < 0) {
      return 0;
    }
    return std::sqrt(prev_var.y);
  }
  // returns 1 for positive peak, -1 for negative peak, 0 for no peak [ for gyro y only ]
  int peak_detection_y() {
    float difference = avg.y - (prev_avg.y);
    if (hit_peak_y && natural_gait(avg.y, delta.y) && std::abs(difference) > THRESHOLD * std::sqrt(prev_var.y)) {
      if (difference > 0) {
        if (last_pos < last_neg && last_pos < second_last_neg) { // WALKING FULL GAIT
          steps += 2;
        }
        stance_percent = std::max(0.0f, std::min(1.0f, (float)(last_neg - last_pos) / (float)(num_readings - last_pos)));
        neg_peak_ratio = std::max(0.0f, std::min(1.0f, second_last_neg_value / last_neg_value));
        last_pos = num_readings;
        hit_peak_y = 0;
        return 1;
      } else {
        second_last_neg_value = last_neg_value;
        last_neg_value = avg.y;
        second_last_neg = last_neg;
        last_neg = num_readings;
        hit_peak_y = 0;
        return -1;
      }
    }
    return 0;
  }
  xyz_t* get_delta() {
    return &delta;
  }
  float get_stance_percent() {
    return stance_percent;
  }
  float get_peak_ratio() {
    return neg_peak_ratio;
  }
  int get_steps() {
    return steps;
  }
};

/* ======================================================================================================================== */
/* ======================================================================================================================== */
/* ======================================================================================================================== */


Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

xyz_t accel;
xyz_t gyro;
euler_t ypr;
quaternion_t rot_vec;
steps_t step_ctr;

window_filter_xyz accel_filter;
window_filter_xyz gyro_filter;

bool mockBNO08X = true;
unsigned long last_reading = 0;
unsigned long gyro_prev = 0;
unsigned long gyro_update = 0;


String getAccelJsonString() {
    return "{\"label\":\"Acceleration\",\"data\":{\"x\":" + String(accel.x) + ",\"y\":" + String(accel.y) + ",\"z\":" + String(accel.z) + "}}";
}

String getGyroJsonString() {
    return "{\"label\":\"Gyroscope\",\"data\":{\"x\":" + String(gyro.x) + ",\"y\":" + String(gyro.y) + ",\"z\":" + String(gyro.z) + "},\"bounds\": {\"minimum\": -10, \"maximum\": 10}}";
}

String getEulerJsonString() {
    return "{\"label\":\"Euler Angles\",\"data\":{\"yaw\":" + String(ypr.yaw) + ",\"pitch\":" + String(ypr.pitch) + ",\"roll\":" + String(ypr.roll) + "}}";
}

String getQuaternionJsonString() {
    return "{\"label\":\"Rotation Vector\",\"data\":{\"real\":" + String(rot_vec.real) + ",\"i\":" + String(rot_vec.i) + ",\"j\":" + String(rot_vec.j) + ",\"k\":" + String(rot_vec.k) + "}}";
}

String getStepsJsonString() {
    return "{\"label\":\"Step Counter\",\"data\":{\"steps\":" + String(gyro_filter.get_steps()) + "}}";
}

String getGyroFilterJsonString() {
    return "{\"label\":\"Gyroscope Filtered\",\"data\":{\"y\":" + String(gyro_filter.get_avg()->y) + "},\"bounds\": {\"minimum\": -10, \"maximum\": 10}}";
}

String getGaitAnalysisJsonString() {
    return "{\"label\":\"Gait Analysis\",\"data\":{\"stancePercent\": " +  String(gyro_filter.get_stance_percent()) + ",\"peakRatio\": " + String(gyro_filter.get_peak_ratio()) + "}}";
}

String limpClass() {
  if (std::abs(gyro_filter.get_avg()->y) < 0.4 || flex_filter.get_var() < 0.005) {
    return String("\"Start moving to get your limp classification!\"");
  } else if (flex_filter.limp_classification()) {
    return String("\"WARNING: You are limping. Correct your gait!\"");
  } else {
    return String("\"Normal gait patterns!!\"");
  }
}
 
String getLimpClassificationText() {
  return "{\"label\":\"Limp Gait Classification\",\"data\":{\"Limp Gait Classification\":" + limpClass() + "}}";
}

void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION, REPORT_RATE_US)) {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED, REPORT_RATE_US)) {
    Serial.println("Could not enable gyroscope");
  }
  if (!bno08x.enableReport(SH2_STEP_COUNTER, INTER_RATE_US)) {
    Serial.println("Could not enable step counter");
  }
  if (!bno08x.enableReport(SH2_ARVR_STABILIZED_RV, ARVR_ROTVEC_US)) {
    Serial.println("Could not enable stabilized remote vector");
  }
  Serial.println("Reading events");
}

void getSensorData() {
  if (bno08x.getSensorEvent(&sensorValue)) {
    switch (sensorValue.sensorId) {
      case SH2_GYROSCOPE_CALIBRATED:
        gyro.set(sensorValue.un.gyroscope.x, sensorValue.un.gyroscope.y, sensorValue.un.gyroscope.z);
        gyro_filter.update(&gyro);
        gyro_prev = gyro_update;
        gyro_update = millis();
        break;
      case SH2_LINEAR_ACCELERATION:
        accel.set(sensorValue.un.linearAcceleration.x, sensorValue.un.linearAcceleration.y, sensorValue.un.linearAcceleration.z);
        accel_filter.update(&accel);
        break;
      case SH2_STEP_COUNTER:
        step_ctr.set(sensorValue.un.stepCounter.steps, sensorValue.un.stepCounter.latency);
        break;
      case SH2_ARVR_STABILIZED_RV:
        quaternionToEulerRV(&sensorValue.un.arvrStabilizedRV, &ypr);
        break;
    }
  }
}

unsigned long last = 0;

void bno08XSetup() {
    if (!bno08x.begin_I2C()) {
        Serial.println("Failed to find BNO08x chip, will mock instead");
//        while (1) { delay(10); }
    } else {
        mockBNO08X = false;
        Serial.println("BNO08x Found!");
        setReports();
        Serial.println("Reading events");
        delay(100);
        last_reading = millis();
    }
}

void bno08XLoop() {
    // delay(10);
    unsigned long curr = millis();
    if (curr - last >= UPDATE_RATE_MS - UPDATE_RATE_CORRECTION) {
        gyro_filter.peak_detection_y();
//        Serial.print("\t"); Serial.print(curr - last_reading);
//        Serial.print("\t"); Serial.print(gyro_update - gyro_prev);
//        Serial.print("\t"); Serial.print(ypr.yaw);
//        Serial.print("\t"); Serial.print(gyro_filter.get_avg()->y); // Rotational acceleration on y-axis
//        Serial.print("\t"); Serial.print(gyro_filter.get_prev_std_y() * THRESHOLD); // Rotational acceleration on y-axis
//        Serial.print("\t"); Serial.print(-1 * (gyro_filter.get_prev_std_y() * THRESHOLD)); // Rotational acceleration on y-axis
//        Serial.print("\t"); Serial.print(gyro_filter.peak_detection_y() * 5); // Rotational acceleration on y-axis
//        Serial.print("\t"); Serial.print(gyro_filter.get_stance_percent());
//        Serial.print("\t"); Serial.print(gyro_filter.get_peak_ratio()); //too noisy, too sporadic...
//        Serial.print("\t"); Serial.print(magnitude(accel_filter.get_avg(), true));
//        Serial.print("\t"); Serial.print(gyro_filter.get_steps());
//        Serial.print("\t"); Serial.print(get_flex_reading());
//        Serial.print("\t"); Serial.print(flex_filter.get_var());
//        
//        Serial.print("\t"); Serial.print(activity.getMostLikelyActivity());
//        Serial.print("\t"); Serial.print(gyro_filter.get_delta()->y);
//        Serial.println();
    }
    if (!mockBNO08X) {
        getSensorData();
    } else {
        gyro.x = 10 * sin(curr);
        gyro.y = 10 * sin(curr + PI / 4);
        gyro.z = 10 * cos(curr);
        step_ctr.steps += millis() / 800;
    }
    if (bno08x.wasReset()) {
        Serial.print("sensor was reset ");
        setReports();
    }
}
