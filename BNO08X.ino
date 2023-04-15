// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9
// For SPI mode, we also need a RESET
// #define BNO08X_RESET 5
// but not for I2C or UART
#define BNO08X_RESET -1

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

// 1000 Hz standard update rate
#define REPORT_RATE_US 500
// 10 Hz interpretation rate
#define INTER_RATE_US 100000
// 400 Hz YPR update rate (do we need this?)
#define ARVR_ROTVEC_US 2500
// 100 Hz serial output rate
#define UPDATE_RATE_MS 10
#define UPDATE_RATE_CORRECTION 2

struct xyz_t {
    float x, y, z;
    xyz_t(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
    xyz_t& operator=(const xyz_t& val) {
        x = val.x;
        y = val.y;
        z = val.z;
    }
    xyz_t operator+(const xyz_t& val) const { return xyz_t(x + val.x, y + val.y, z + val.z); }
    xyz_t operator-(const xyz_t& val) const { return xyz_t(x - val.x, y - val.y, z - val.z); }
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

struct euler_t {
    float yaw, pitch, roll;
    euler_t(float yaw = 0.0f, float pitch = 0.0f, float roll = 0.0f) : yaw(yaw), pitch(pitch), roll(roll) {}
};

struct quaternion_t {
    float real, i, j, k;
    quaternion_t(float real = 0.0f, float i = 0.0f, float j = 0.0f, float k = 0.0f) : real(real), i(i), j(j), k(k) {}
};

struct steps_t {
    int steps;
    float latency;
    steps_t(int steps = 0, float latency = 0.0f) : steps(steps), latency(latency) {}
};

struct activity_t {
    char* mostLikely;
    // float unknownConf, inVehicleConf, onBicycleConf, onFootConf, stillConf, tiltingConf, walkingConf, runningConf, onStairsConf;
};

float magnitude(xyz_t* input, bool sqrt = false) {
    float sum = sq(input->x) + sq(input->y) + sq(input->z);
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
}

void quaternionToEulerRV(sh2_RotationVectorWAcc_t* rotational_vector, euler_t* ypr, bool degrees = false) {
    quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
}

void quaternionToEulerGI(sh2_GyroIntegratedRV_t* rotational_vector, euler_t* ypr, bool degrees = false) {
    quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
}

xyz_t accel;
xyz_t gyro;
euler_t ypr;
quaternion_t rot_vec;
steps_t step_ctr;
activity_t activity;

bool mockBNO08X = true;

String getAccelJsonString() {
    return "{\"label\":\"Acceleration\",\"data\":{\"x\":" + String(accel.x) + ",\"y\":" + String(accel.y) + ",\"z\":" + String(accel.z) + "}}";
}

String getGyroJsonString() {
    return "{\"label\":\"Gyroscope\",\"data\":{\"px\":" + String(gyro.x) + ",\"py\":" + String(gyro.y) + ",\"pz\":" + String(gyro.z) + "},\"bounds\": {\"minimum\": -10, \"maximum\": 10}}";
}

String getEulerJsonString() {
    return "{\"label\":\"Euler Angles\",\"data\":{\"yaw\":" + String(ypr.yaw) + ",\"pitch\":" + String(ypr.pitch) + ",\"roll\":" + String(ypr.roll) + "}}";
}

String getQuaternionJsonString() {
    return "{\"label\":\"Rotation Vector\",\"data\":{\"real\":" + String(rot_vec.real) + ",\"i\":" + String(rot_vec.i) + ",\"j\":" + String(rot_vec.j) + ",\"k\":" + String(rot_vec.k) + "}}";
}

String getStepsJsonString() {
    return "{\"label\":\"Step Counter\",\"data\":{\"steps\":" + String(step_ctr.steps) + ",\"latency\":" + String(step_ctr.latency) + "}}";
}

String getActivityJsonString() {
    return "{\"label\":\"Activity Classification\",\"data\":{\"mostLikely\":\"" + String(activity.mostLikely) + "\"}}";
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
    if (!bno08x.enableReport(SH2_PERSONAL_ACTIVITY_CLASSIFIER, INTER_RATE_US)) {
        Serial.println("Could not enable personal activity classifier");
    }
    if (!bno08x.enableReport(SH2_ARVR_STABILIZED_RV, ARVR_ROTVEC_US)) {
        Serial.println("Could not enable stabilized remote vector");
    }
}

void getMostLikelyActivity(uint8_t activity_id) {
    switch (activity_id) {
        case PAC_UNKNOWN:
            activity.mostLikely = "Unknown";
            break;
        case PAC_IN_VEHICLE:
            activity.mostLikely = "In Vehicle";
            break;
        case PAC_ON_BICYCLE:
            activity.mostLikely = "On Bicycle";
            break;
        case PAC_ON_FOOT:
            activity.mostLikely = "On Foot";
            break;
        case PAC_STILL:
            activity.mostLikely = "Still";
            break;
        case PAC_TILTING:
            activity.mostLikely = "Tilting";
            break;
        case PAC_WALKING:
            activity.mostLikely = "Walking";
            break;
        case PAC_RUNNING:
            activity.mostLikely = "Running";
            break;
        case PAC_ON_STAIRS:
            activity.mostLikely = "On Stairs";
            break;
        default:
            activity.mostLikely = "NOT LISTED";
    }
}

void getSensorData() {
    if (bno08x.getSensorEvent(&sensorValue)) {
        switch (sensorValue.sensorId) {
            case SH2_GYROSCOPE_CALIBRATED:
                gyro.x = sensorValue.un.gyroscope.x;
                gyro.y = sensorValue.un.gyroscope.y;
                gyro.z = sensorValue.un.gyroscope.z;
                break;
            case SH2_LINEAR_ACCELERATION:
                accel.x = sensorValue.un.linearAcceleration.x;
                accel.y = sensorValue.un.linearAcceleration.y;
                accel.z = sensorValue.un.linearAcceleration.z;
                break;
            case SH2_STEP_COUNTER:
                step_ctr.steps = sensorValue.un.stepCounter.steps;
                step_ctr.latency = sensorValue.un.stepCounter.latency;
                break;
            case SH2_ARVR_STABILIZED_RV:
                quaternionToEulerRV(&sensorValue.un.arvrStabilizedRV, &ypr);
                break;
            case SH2_PERSONAL_ACTIVITY_CLASSIFIER:
                getMostLikelyActivity(sensorValue.un.personalActivityClassifier.mostLikelyState);
                /*
                activity.unknownConf = sensorValue.un.personalActivityClassifier.confidence[PAC_UNKNOWN];
                activity.inVehicleConf = sensorValue.un.personalActivityClassifier.confidence[PAC_IN_VEHICLE];
                activity.onBicycleConf = sensorValue.un.personalActivityClassifier.confidence[PAC_ON_BICYCLE];
                activity.onFootConf = sensorValue.un.personalActivityClassifier.confidence[PAC_ON_FOOT];
                activity.stillConf = sensorValue.un.personalActivityClassifier.confidence[PAC_STILL];
                activity.tiltingConf = sensorValue.un.personalActivityClassifier.confidence[PAC_TILTING];
                activity.walkingConf = sensorValue.un.personalActivityClassifier.confidence[PAC_WALKING];
                activity.runningConf = sensorValue.un.personalActivityClassifier.confidence[PAC_RUNNING];
                activity.onStairsConf = sensorValue.un.personalActivityClassifier.confidence[PAC_ON_STAIRS];
                */
                break;
        }
    }
}

unsigned long last = 0;

void bno08XSetup() {
    if (!bno08x.begin_I2C()) {
        Serial.println("Failed to find BNO08x chip, will mock instead");
    } else {
        mockBNO08X = false;
        Serial.println("BNO08x Found!");
        setReports();
        Serial.println("Reading events");
        delay(100);
        last = millis();
    }
}

void bno08XLoop() {
    // delay(10);
    unsigned long curr = millis();
    if (curr - last >= UPDATE_RATE_MS - UPDATE_RATE_CORRECTION) {
        /*
        Serial.print(curr - last);
        Serial.print("\t");
        Serial.print(ypr.yaw);
        Serial.print("\t");
        Serial.print(gyro.y);  // Rotational acceleration on y-axis
        Serial.print("\t");
        Serial.println(magnitude(&accel, true));
        last = curr;
        */
    }
    if (!mockBNO08X) {
        getSensorData();
    } else {
        gyro.x = 10 * sin(curr);
        gyro.y = 10 * sin(curr + PI / 4);
        gyro.z = 10 * cos(curr);
        step_ctr.steps += millis() / 800;
        getMostLikelyActivity((uint8_t) random(PAC_UNKNOWN+1, PAC_ON_STAIRS+1));
    }
    if (bno08x.wasReset()) {
        Serial.print("sensor was reset ");
        setReports();
    }
}
