Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

struct xyz_t
{
  float x;
  float y;
  float z;
};

struct xyz_t accel;
struct xyz_t gyro;
struct xyz_t mag_field;
struct xyz_t lin_accel;
struct xyz_t gravity;
struct xyz_t raw_accel;
struct xyz_t raw_gyro;
struct xyz_t raw_mag_field;
String xyzToJsonString(String label, struct xyz_t *v)
{
  return "{\"label\":\"" + label + "\",\"data\":{\"x\":" + String(v->x) + ",\"y\":" + String(v->y) + ",\"z\":" + String(v->z) + "}}";
}
String getAccelJsonString()
{
  return xyzToJsonString("Accelerometer", &accel);
}
String getGyroJsonString()
{
  return xyzToJsonString("Gyroscope", &gyro);
}
String getMagFieldJsonString()
{
  return xyzToJsonString("Magnetic Field", &mag_field);
}
String getLinAccelJsonString()
{
  return xyzToJsonString("Linear Acceleration", &lin_accel);
}
String getGravityJsonString()
{
  return xyzToJsonString("Gravity", &gravity);
}
String getRawAccelJsonString()
{
  return xyzToJsonString("Raw Accelerometer", &raw_accel);
}
String getRawGyroJsonString()
{
  return xyzToJsonString("Raw Gyroscope", &raw_gyro);
}
String getRawMagFieldJsonString()
{
  return xyzToJsonString("Raw Magnetic Field", &raw_mag_field);
}

struct quaternion_t
{
  float real;
  float i;
  float j;
  float k;
};
struct quaternion_t rot_vec;
struct quaternion_t geo_mag_rot_vec;
struct quaternion_t game_rot_vec;
String quaternionToJsonString(String label, struct quaternion_t *q)
{
  return "{\"label\":\"" + label + "\",\"data\":{\"real\":" + String(q->real) + ",\"i\":" + String(q->i) + ",\"j\":" + String(q->j) + ",\"k\":" + q->k + "}}";
}
String getRotVecJsonString()
{
  return quaternionToJsonString("Rotation Vector", &rot_vec);
}
String getGeoMagRotVecJsonString()
{
  return quaternionToJsonString("Geomagnetic Rotation Vector", &geo_mag_rot_vec);
}
String getGameRotVecJsonString()
{
  return quaternionToJsonString("Game Rotation Vector", &game_rot_vec);
}

struct euler_t
{
  float yaw;
  float pitch;
  float roll;
} ypr;
String getEulerJsonString()
{
  return "{\"label\":\"Euler Angles\",\"data\":{\"yaw\":" + String(ypr.yaw) + ",\"pitch\":" + String(ypr.pitch) + ",\"roll\":" + String(ypr.roll) + "}}";
}

struct steps_t
{
  int steps;
  float latency;
} steps;
String getStepsJsonString()
{
  return "{\"label\":\"Step Counter\",\"data\":{\"steps\":" + String(steps.steps) + ",\"latency\":" + String(steps.latency) + "}}";
}

struct stability_t
{
  char *classification;
} stability;
String getStabilityJsonString()
{
  return "{\"label\":\"Stability Classification\",\"data\":{\"classification\":\"" + String(stability.classification) + "\"}}";
}

struct shake_t
{
  char axis;
} shake;
String getShakeJsonString()
{
  return "{\"label\":\"Shake Detection Axis\",\"data\":{\"axis\":\"" + String(shake.axis) + "\"}}";
}

struct activity_t
{
  char *mostLikely;
  float unknownConf;
  float inVehicleConf;
  float onBicycleConf;
  float onFootConf;
  float stillConf;
  float tiltingConf;
  float walkingConf;
  float runningConf;
  float onStairsConf;
} activity;
String getActivityJsonString()
{
  return "{\"label\":\"Activity Classification\",\"data\":{\"mostLikely\":" + String(activity.mostLikely) + "\"}}}";
}

bool mockBNO08X = true;
String getMockJsonString() {
  struct xyz_t mockXYZ = {random(-10, 10), random(-10, 10), random(-10, 10)};
  struct quaternion_t mockQ = {random(-10, 10), random(-10, 10), random(-10, 10), random(-10, 10)};

  String mockXYZJsonString = xyzToJsonString("XYZ", &mockXYZ);
  String mockQJsonString = quaternionToJsonString("Quaternion", &mockQ);
  return "[" + mockXYZJsonString + "," + mockQJsonString + "]";
}


void setReports(void)
{
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER))
  {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED))
  {
    Serial.println("Could not enable gyroscope");
  }
  if (!bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED))
  {
    Serial.println("Could not enable magnetic field calibrated");
  }
  /*
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION))
  {
    Serial.println("Could not enable linear acceleration");
  }
  if (!bno08x.enableReport(SH2_GRAVITY))
  {
    Serial.println("Could not enable gravity vector");
  }
  if (!bno08x.enableReport(SH2_ROTATION_VECTOR))
  {
    Serial.println("Could not enable rotation vector");
  }
  if (!bno08x.enableReport(SH2_GEOMAGNETIC_ROTATION_VECTOR))
  {
    Serial.println("Could not enable geomagnetic rotation vector");
  }
  if (!bno08x.enableReport(SH2_GAME_ROTATION_VECTOR))
  {
    Serial.println("Could not enable game rotation vector");
  }
  */
  if (!bno08x.enableReport(SH2_STEP_COUNTER))
  {
    Serial.println("Could not enable step counter");
  }
  if (!bno08x.enableReport(SH2_STABILITY_CLASSIFIER))
  {
    Serial.println("Could not enable stability classifier");
  }
  /*
  if (!bno08x.enableReport(SH2_RAW_ACCELEROMETER))
  {
    Serial.println("Could not enable raw accelerometer");
  }
  if (!bno08x.enableReport(SH2_RAW_GYROSCOPE))
  {
    Serial.println("Could not enable raw gyroscope");
  }
  if (!bno08x.enableReport(SH2_RAW_MAGNETOMETER))
  {
    Serial.println("Could not enable raw magnetometer");
  }
  if (!bno08x.enableReport(SH2_SHAKE_DETECTOR))
  {
    Serial.println("Could not enable shake detector");
  }
  */
  if (!bno08x.enableReport(SH2_PERSONAL_ACTIVITY_CLASSIFIER))
  {
    Serial.println("Could not enable personal activity classifier");
  }
  if (!bno08x.enableReport(SH2_ARVR_STABILIZED_RV, 5000))
  {
    Serial.println("Could not enable stabilized remote vector");
  }
}

void getMostLikelyActivity(uint8_t activity_id)
{
  switch (activity_id)
  {
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

void quaternionToEuler(float qr, float qi, float qj, float qk, euler_t *ypr, bool degrees = false)
{

  float sqr = sq(qr);
  float sqi = sq(qi);
  float sqj = sq(qj);
  float sqk = sq(qk);

  ypr->yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr));
  ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
  ypr->roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));

  if (degrees)
  {
    ypr->yaw *= RAD_TO_DEG;
    ypr->pitch *= RAD_TO_DEG;
    ypr->roll *= RAD_TO_DEG;
  }
}

void getSensorData()
{
  if (!bno08x.getSensorEvent(&sensorValue))
  {
    return;
  }
  switch (sensorValue.sensorId)
  {
  case SH2_ACCELEROMETER:
    accel.x = sensorValue.un.accelerometer.x;
    accel.y = sensorValue.un.accelerometer.y;
    accel.z = sensorValue.un.accelerometer.z;
    break;
  case SH2_GYROSCOPE_CALIBRATED:
    gyro.x = sensorValue.un.gyroscope.x;
    gyro.y = sensorValue.un.gyroscope.y;
    gyro.z = sensorValue.un.gyroscope.z;
    break;
  case SH2_MAGNETIC_FIELD_CALIBRATED:
    mag_field.x = sensorValue.un.magneticField.x;
    mag_field.y = sensorValue.un.magneticField.y;
    mag_field.z = sensorValue.un.magneticField.z;
    break;
  case SH2_LINEAR_ACCELERATION:
    lin_accel.x = sensorValue.un.linearAcceleration.x;
    lin_accel.y = sensorValue.un.linearAcceleration.y;
    lin_accel.z = sensorValue.un.linearAcceleration.z;
    break;
  case SH2_GRAVITY:
    gravity.x = sensorValue.un.gravity.x;
    gravity.y = sensorValue.un.gravity.y;
    gravity.z = sensorValue.un.gravity.z;
    break;
    //
  case SH2_ROTATION_VECTOR:
    rot_vec.real = sensorValue.un.rotationVector.real;
    rot_vec.i = sensorValue.un.rotationVector.i;
    rot_vec.j = sensorValue.un.rotationVector.j;
    rot_vec.k = sensorValue.un.rotationVector.k;
    break;
  case SH2_GEOMAGNETIC_ROTATION_VECTOR:
    geo_mag_rot_vec.real = sensorValue.un.geoMagRotationVector.real;
    geo_mag_rot_vec.i = sensorValue.un.geoMagRotationVector.i;
    geo_mag_rot_vec.j = sensorValue.un.geoMagRotationVector.j;
    geo_mag_rot_vec.k = sensorValue.un.geoMagRotationVector.k;
    break;
  case SH2_GAME_ROTATION_VECTOR:
    game_rot_vec.real = sensorValue.un.gameRotationVector.real;
    game_rot_vec.i = sensorValue.un.gameRotationVector.i;
    game_rot_vec.j = sensorValue.un.gameRotationVector.j;
    game_rot_vec.k = sensorValue.un.gameRotationVector.k;
    break;
  case SH2_STEP_COUNTER:
    steps.steps = sensorValue.un.stepCounter.steps;
    steps.latency = sensorValue.un.stepCounter.latency;
    break;
  case SH2_STABILITY_CLASSIFIER:
  {
    switch (sensorValue.un.stabilityClassifier.classification)
    {
    case STABILITY_CLASSIFIER_UNKNOWN:
      stability.classification = "Unknown";
      break;
    case STABILITY_CLASSIFIER_ON_TABLE:
      stability.classification = "On Table";
      break;
    case STABILITY_CLASSIFIER_STATIONARY:
      stability.classification = "Stationary";
      break;
    case STABILITY_CLASSIFIER_STABLE:
      stability.classification = "Stable";
      break;
    case STABILITY_CLASSIFIER_MOTION:
      stability.classification = "In Motion";
      break;
    }
  }
  case SH2_RAW_ACCELEROMETER:
    raw_accel.x = sensorValue.un.rawAccelerometer.x;
    raw_accel.y = sensorValue.un.rawAccelerometer.y;
    raw_accel.z = sensorValue.un.rawAccelerometer.z;
    break;
    // return "{\"rawAccelerometer\":{\"x\":" + sensorValue.un.rawAccelerometer.x + ",\"y\":" + sensorValue.un.rawAccelerometer.y + ",\"z\":" + sensorValue.rawAccelerometer.z + "}}";
  case SH2_RAW_GYROSCOPE:
    raw_gyro.x = sensorValue.un.rawGyroscope.x;
    raw_gyro.y = sensorValue.un.rawGyroscope.y;
    raw_gyro.z = sensorValue.un.rawGyroscope.z;
    break;
  case SH2_RAW_MAGNETOMETER:
    raw_mag_field.x = sensorValue.un.rawMagnetometer.x;
    raw_mag_field.y = sensorValue.un.rawMagnetometer.y;
    raw_mag_field.z = sensorValue.un.rawMagnetometer.z;
    break;
  case SH2_SHAKE_DETECTOR:
  {
    switch (sensorValue.un.shakeDetector.shake)
    {
    case SHAKE_X:
      shake.axis = 'X';
      break;
    case SHAKE_Y:
      shake.axis = 'Y';
      break;
    case SHAKE_Z:
      shake.axis = 'Z';
      break;
    default:
      return;
    }
  }
  case SH2_PERSONAL_ACTIVITY_CLASSIFIER:
  {
    getMostLikelyActivity(sensorValue.un.personalActivityClassifier.mostLikelyState);
    activity.unknownConf = sensorValue.un.personalActivityClassifier.confidence[PAC_UNKNOWN];
    activity.inVehicleConf = sensorValue.un.personalActivityClassifier.confidence[PAC_IN_VEHICLE];
    activity.onBicycleConf = sensorValue.un.personalActivityClassifier.confidence[PAC_ON_BICYCLE];
    activity.onFootConf = sensorValue.un.personalActivityClassifier.confidence[PAC_ON_FOOT];
    activity.stillConf = sensorValue.un.personalActivityClassifier.confidence[PAC_STILL];
    activity.tiltingConf = sensorValue.un.personalActivityClassifier.confidence[PAC_TILTING];
    activity.walkingConf = sensorValue.un.personalActivityClassifier.confidence[PAC_WALKING];
    activity.runningConf = sensorValue.un.personalActivityClassifier.confidence[PAC_RUNNING];
    activity.onStairsConf = sensorValue.un.personalActivityClassifier.confidence[PAC_ON_STAIRS];
    break;
  }
  case SH2_ARVR_STABILIZED_RV:
  {
    sh2_RotationVectorWAcc_t *rotational_vector = &sensorValue.un.arvrStabilizedRV;
    quaternionToEuler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, &ypr, true);
    break;
  }
  }
}

void bno08XSetup()
{
  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C())
  {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte UART buffer!
    // if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip, will send random values instead");
  }
  else
  {
    mockBNO08X = false;
    Serial.println("BNO08x Found!");
    setReports();
    Serial.println("Reading events");
    delay(100);
  }
}

void bno08XLoop() {
  delay(10);
  if (!mockBNO08X) {
    getSensorData();
  }
}
