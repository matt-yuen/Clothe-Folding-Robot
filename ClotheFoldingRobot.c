//Vishvam Mazumdar, Vishwa Sheth, Matthew Yuen, Aggash Sivasothy
//Clothes Folding Robot Code
#include "EV3Servo-lib-UW.c"

//setup constants
const int SORT_BY_COLOR_BUTTON = buttonLeft,
	SORT_BY_TYPE_BUTTON = buttonRight, SORT_PANT = buttonLeft,
	SORT_SHIRT = buttonRight, WAIT_TIME = 2000;

//Motors
const int LEFT_MOTOR = 6, RIGHT_MOTOR = 7, BACK_FLAP_MOTOR = 3,
	RIGHT_FLAP_MOTOR = 4, LEFT_FLAP_MOTOR = 5, BELT_MOTOR = motorA,
	NORM_MOTOR_SPEED = 30;

//Sensors/ Ports
const int TETRIX_MOTORS_PORT = S1, TOUCH_SENSOR = S2,
	LINE_COLOR_SENSOR = S3, CLOTH_COLOR_SENSOR = S4;

//Dropping off clothing constants
const int TIME_TO_GO_FORWARD = 3200, TIME_TO_DROP_CLOTHING = 4000,
	BELT_ENC_LIMIT = -250;

//Line following constants
const float HIGHER_SPEED_PROP_FRONT = 2.2, LOWER_SPEED_PROP_FRONT = 0,
	LEFT_LINE_COLOR = (int)colorWhite,
	RIGHT_LINE_COLOR = (int)colorGreen,
	LOWER_SPEED_PROP_BACK = 0.4, HIGHER_SPEED_PROP_BACK = 1.5;

//Rotate constants
const int PANT_LINE_COLOR = (int)colorBlue,
	SHIRT_LINE_COLOR = (int) colorRed, ROTATE_SPEED_LEFT = 30,
	ROTATE_SPEED_RIGHT = 80;

//Emergency variables
const int buttonEmergency = buttonEnter;
bool emergency = false;

//Folding Constants
const int FOLD_TIME = 1500, //time it takes for motors to fold flaps
	NULL_POSITION = 90, FOLDED_POSITION = 45;

//functions
void initializeSensorsAndMotors();
void setSpeedPosition(int motor_port, int power_position);
bool readLineColor(int color);
void follow(int motor_power);
bool isEmergency();
void lineFollow(int motor_power, int end_color, int time_to_follow);
void dropClothing();
void rotateToColor( int color);
void foldClothes();

//Everyone
task main()
{
	initializeSensorsAndMotors();

	//welcome user
	playSoundFile("Hello");

	bool sort_by_type = false;

	//Wait for button press
	while(!(isEmergency() || getButtonPress(SORT_BY_COLOR_BUTTON) ||
		 getButtonPress(SORT_BY_TYPE_BUTTON)))
	{}

	//Store data from button press, and alert user what's chosen
	if(getButtonPress(SORT_BY_TYPE_BUTTON))
	{
		sort_by_type = true;
		playSoundFile("Laser");
	}else
	{
		playSoundFile("Color");
	}
	time1[T1] = 0;
	while(!isEmergency() && time1[T1] < WAIT_TIME)
	{}

	//Wait for button release
	while(!isEmergency() && (getButtonPress(SORT_BY_COLOR_BUTTON) ||
		getButtonPress(SORT_BY_TYPE_BUTTON)))
	{}

	//Main loops - One for sorting by type and other for color
	//sort by color code:
	if(!sort_by_type)
	{

		while(!isEmergency())
		{
			playSoundFile("Ready");

			//Wait for user to press go button
			while(!(isEmergency() || SensorValue(TOUCH_SENSOR)))
			{}

			//Wait for user to release go button
			while(!isEmergency() && SensorValue(TOUCH_SENSOR))
			{}
			playSoundFile("Activate");
			//Wait a couple seconds after button release
			time1[T1] = 0;
			while(!isEmergency() && time1[T1] < WAIT_TIME)
			{}

			if(!isEmergency())
			{
				foldClothes();

				//set the beginning and end color of the line to
				//follow depending on what the top sensor reads
				//play appropriate sound based on color
				//sort to yellow line if black or white clothes
				int color = SensorValue[CLOTH_COLOR_SENSOR];
				int end_color = (int)colorRed;
				if(color == (int)colorRed)
				{
					end_color=(int)colorBlue;
					playSoundFile("Red");
				}else if(color==(int)colorWhite)
				{
					color = (int)colorYellow;
					playSoundFile("White");
				}else if (color == (int) colorBlue)
				{
					playSoundFile("Blue");
				}else if(color ==(int)colorBlack)
				{
					color = (int)colorYellow;
					playSoundFile("Black");
				}

				rotateToColor(color);

				//line follow to end of line and then go a bit forward
				//(based on tested time) to drop clothing
				//black passed to line follow when not following to
				//a color, but for a time
				lineFollow(NORM_MOTOR_SPEED, end_color, 0);
				lineFollow(NORM_MOTOR_SPEED, (int)colorBlack, TIME_TO_GO_FORWARD);

				dropClothing();

				//go back to original color, then go forward and back
				//again to properly reallign self
				lineFollow(-NORM_MOTOR_SPEED, color, 0);
				lineFollow(NORM_MOTOR_SPEED,end_color,0);
				lineFollow(-NORM_MOTOR_SPEED, color, 0);

			}
		}
	}else
	{
		//variable for whether it's a shirt or a pant
		int sort_shirt = true;
		while(!isEmergency())
		{
			playSoundFile("Ready");

			//wait for pant or shirt button press and release
			while(!isEmergency() &&!getButtonPress(SORT_PANT) &&
				 !getButtonPress(SORT_SHIRT))
			{}
			if(getButtonPress(SORT_PANT))
			{
				sort_shirt = false;
				playSoundFile("Left");
			}else
			{
				sort_shirt = true;
				playSoundFile("Right");
			}
			while(!isEmergency() && (getButtonPress(SORT_PANT)
				|| getButtonPress(SORT_SHIRT)))
			{}

			//Wait for user to release go button
			while(!(isEmergency() || SensorValue(TOUCH_SENSOR)))
			{}
			while(!isEmergency() && SensorValue(TOUCH_SENSOR))
			{}
			playSoundFile("Activate");
			time1[T1] = 0;
			while(!isEmergency() && time1[T1] < WAIT_TIME)
			{}

			if(!isEmergency())
			{
				foldClothes();
				int color = 0, end_color = 0;
				if(sort_shirt)
				{
					color = SHIRT_LINE_COLOR;
					end_color = (int)colorBlue;
				}
				else
				{
					color = PANT_LINE_COLOR;
					end_color = (int)colorRed;
				}

				//same as color code
				rotateToColor(color);
				lineFollow(NORM_MOTOR_SPEED, end_color, 0);
				lineFollow(NORM_MOTOR_SPEED, (int)colorBlack, TIME_TO_GO_FORWARD);


				dropClothing();
				lineFollow(-NORM_MOTOR_SPEED, color, 0);
				lineFollow(NORM_MOTOR_SPEED, end_color,0);
				lineFollow(-NORM_MOTOR_SPEED, color, 0);

				setSpeedPosition(RIGHT_MOTOR,0);
				setSpeedPosition(LEFT_MOTOR,0);
			}


		}


	}

}

//Matthew
//initializes
void initializeSensorsAndMotors()
{
	//sensors
	SensorType[LINE_COLOR_SENSOR] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[LINE_COLOR_SENSOR] = modeEV3Color_Color;
	wait1Msec(50);

	SensorType[CLOTH_COLOR_SENSOR] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[CLOTH_COLOR_SENSOR] = modeEV3Color_Color;
	wait1Msec(50);

	SensorType[TOUCH_SENSOR] = sensorEV3_Touch;

	//motors - set all motors to default positions
	//e.g. chassis motors to still, right, left, and back flap motors
	//flat ready for cloth to be placed on
	SensorType[TETRIX_MOTORS_PORT] = sensorI2CCustom9V;
	setSpeedPosition(RIGHT_MOTOR,0);
	setSpeedPosition(LEFT_MOTOR,0);
	setSpeedPosition(RIGHT_FLAP_MOTOR,-NULL_POSITION);
	setSpeedPosition(LEFT_FLAP_MOTOR,NULL_POSITION);
	setSpeedPosition(BACK_FLAP_MOTOR,NULL_POSITION);

}

//Vishvam
//code to set speed of continuous motors or position of servos
void setSpeedPosition(int motor_port, int power_position)
{
	//for servos
	if(motor_port == RIGHT_FLAP_MOTOR ||
		motor_port== LEFT_FLAP_MOTOR ||
		motor_port == BACK_FLAP_MOTOR)
	{
		setServoPosition(TETRIX_MOTORS_PORT, motor_port,
			power_position);
	}else//for continuous
	{
		setServoSpeed(TETRIX_MOTORS_PORT, motor_port ,
			power_position);
	}
}

//Aggash
//function that returns if the color of a line has been reached
//initially implemented to check if the average measurements over time
//period were the color passed in
bool readLineColor(int color)
{
	/*
	time1[T3] = 0;
	int sum = 0;
	int num = 0;
	while(time1[T3]<10)
	{
		sum+=SensorValue[LINE_COLOR_SENSOR];
		num++;
	}

	return (int)ceil(1.0*sum/num) == color;
	*/
	return SensorValue[LINE_COLOR_SENSOR] == color;
}

//Vishwa
//checks if the emergency button is pressed, or the global emergency
//variable is true
bool isEmergency()
{
	if(getButtonPress(buttonEmergency)||emergency)
	{
		emergency = true;
	}
	return emergency;
}

//Vishvam
//implements code for following a line for set time or to end color
//calls follow
//time 0 must be passed in to follow by color
//and colorBlack (as a convention) if followed for time
void lineFollow(int motor_power, int end_color, int time_to_follow)
{
	if(time_to_follow)
	{
		time1[T1] = 0;
		while(!isEmergency()&&time1[T1]<time_to_follow)
		{
			follow(motor_power);
		}
	}else
	{
		//when following to a color, waits till it sees 2 in a row of
		//color: used to prevent reading wrong colors on boundary
		//lines
		int count = 0;
		int max_count = 2;
		while(!isEmergency()&&(count<max_count))
		{
			if(readLineColor(end_color))
				count++;
			else
				count = 0;
			follow(motor_power);
		}

	}
	setSpeedPosition(LEFT_MOTOR, 0);
	setSpeedPosition(RIGHT_MOTOR, 0);

}

//Vishvam
//follows a line, sequence changes if going backwards or going forward
void follow(int motor_power)
{
	//forward code: if right color read, make right motor go faster,
	//if left color read, make left motor go faster
	if(motor_power>=0)
	{
		if(readLineColor(LEFT_LINE_COLOR))
		{
			setSpeedPosition(LEFT_MOTOR, -1*motor_power*
				HIGHER_SPEED_PROP_FRONT);
			setSpeedPosition(RIGHT_MOTOR, motor_power*
				LOWER_SPEED_PROP_FRONT);
		}else if(readLineColor(RIGHT_LINE_COLOR))
		{
			setSpeedPosition(LEFT_MOTOR, -1*motor_power*
				LOWER_SPEED_PROP_FRONT);
			setSpeedPosition(RIGHT_MOTOR, motor_power*
				HIGHER_SPEED_PROP_FRONT);
		}else
		{
			setSpeedPosition(LEFT_MOTOR, -1*motor_power);
			setSpeedPosition(RIGHT_MOTOR, motor_power);
		}
	}else
	//backward code: changes proportions and which side goes faster
	//in diff scenarios; if right line read, make left go faster
	//if left line read, make right go faster
	{
		if(readLineColor(RIGHT_LINE_COLOR))
		{
			setSpeedPosition(LEFT_MOTOR, -1*motor_power*
				HIGHER_SPEED_PROP_BACK);
			setSpeedPosition(RIGHT_MOTOR, 1*motor_power*
				LOWER_SPEED_PROP_BACK);
		}else if(readLineColor(LEFT_LINE_COLOR))
		{
			setSpeedPosition(LEFT_MOTOR, -1*motor_power*
				HIGHER_SPEED_PROP_BACK);
			setSpeedPosition(RIGHT_MOTOR,1* motor_power*
				LOWER_SPEED_PROP_BACK);
		}else
		{
			setSpeedPosition(LEFT_MOTOR, -1*motor_power);
			setSpeedPosition(RIGHT_MOTOR, motor_power);
		}
	}
}

//Aggash
//To drop clothing, run belt motor first, then
// spin belt while following
//backward for set time
void dropClothing()
{
	nMotorEncoder[BELT_MOTOR] = 0;
	motor[BELT_MOTOR] = -NORM_MOTOR_SPEED;
	while(nMotorEncoder[BELT_MOTOR]>BELT_ENC_LIMIT)
	{}

	lineFollow(-NORM_MOTOR_SPEED, (int)colorBlack, TIME_TO_DROP_CLOTHING);

	motor[BELT_MOTOR] = 0;
}

//Vishwa
void rotateToColor( int color)
{
	//the two motors spin at different speeds to rotate approximately
	//back axis with minimal slipping - tested values
	setSpeedPosition(LEFT_MOTOR, ROTATE_SPEED_LEFT);
	setSpeedPosition(RIGHT_MOTOR, ROTATE_SPEED_RIGHT);

	//wait till color has been seen a certian number of times
	//number depends on color, see tested values below
	int count = 0;
	int max_count = 75;
	if(color == (int)colorYellow)
		max_count = 300;
	while(!isEmergency()&&(count<max_count||!readLineColor(color)))
	{
		if(readLineColor(color))
			count++;
		else
			count = 0;
	}

	//Say thank you once line is found
	playSoundFile("Thank you");
	setSpeedPosition(LEFT_MOTOR, 0);
	setSpeedPosition(RIGHT_MOTOR, 0);

}

//Matthew
void foldClothes()
{

	time1[T4] = 0;
	while(!isEmergency() && time1[T4] < FOLD_TIME)
	{}

	//fold left flap and bring back to original pos.
	setSpeedPosition(LEFT_FLAP_MOTOR, -FOLDED_POSITION);
	time1[T4] = 0;
	while(!isEmergency() && time1[T4] < FOLD_TIME)
	{}
	setSpeedPosition(LEFT_FLAP_MOTOR, NULL_POSITION);
	time1[T4] = 0;
	while(!isEmergency() && time1[T4] < FOLD_TIME)
	{}

	//fold right flap and bring back to original pos
	setSpeedPosition(RIGHT_FLAP_MOTOR, FOLDED_POSITION);
	time1[T4] = 0;
	while(!isEmergency() && time1[T4] < FOLD_TIME)
	{}
	setSpeedPosition(RIGHT_FLAP_MOTOR, -NULL_POSITION);
	time1[T4] = 0;
	while(!isEmergency() && time1[T4] < FOLD_TIME)
	{}

	//fold back flap and bring back to original pos
	setSpeedPosition(BACK_FLAP_MOTOR, -FOLDED_POSITION);
	time1[T4] = 0;
	while(!isEmergency() && time1[T4] < FOLD_TIME)
	{}
	setSpeedPosition(BACK_FLAP_MOTOR, NULL_POSITION);
	time1[T4] = 0;
	while(!isEmergency() && time1[T4] < FOLD_TIME)
	{}
}
