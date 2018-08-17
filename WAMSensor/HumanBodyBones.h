enum Bones
{
  RIGHT_UPPER_ARM = 55,
  RIGHT_LOWER_ARM = 56,
  LEFT_UPPER_ARM = 57,
  LEFT_LOWER_ARM = 58, 
  CHEST = 59,
  HEAD = 60
};

char* getSensorTopicNameByID(int sensorId) {
  switch(sensorId)
  {
    case Bones::HEAD: 
      return "Head";
      break;
    case Bones::CHEST: 
      return "Chest";
      break;
    case Bones::RIGHT_UPPER_ARM: 
      return "RightUpperArm";
      break;
    case Bones::RIGHT_LOWER_ARM: 
      return "RightLowerArm";
      break;
    case Bones::LEFT_UPPER_ARM: 
      return "LeftUpperArm";
      break;
    case Bones::LEFT_LOWER_ARM: 
      return "LeftLowerArm";
      break;
    default: 
      return "Head";
      break;
  }
}
