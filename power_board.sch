EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:power_board-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Q_NMOS_DGS Q?
U 1 1 59D313BD
P 1550 1700
F 0 "Q?" H 1750 1750 50  0000 L CNN
F 1 "Q_NMOS_DGS" H 1750 1650 50  0000 L CNN
F 2 "" H 1750 1800 50  0001 C CNN
F 3 "" H 1550 1700 50  0001 C CNN
F 4 "Rohm Semiconductor" H 1550 1700 60  0001 C CNN "Manufacturer"
F 5 "RU1J002YNTCL" H 1550 1700 60  0001 C CNN "Manufacturer PN"
F 6 "50V" H 1550 1700 60  0001 C CNN "Vds"
F 7 "0.9, 4.5V" H 1550 1700 60  0001 C CNN "Drive Voltage"
F 8 "2.2Ohm" H 1550 1700 60  0001 C CNN "Rds on"
F 9 "200mA" H 1550 1700 60  0001 C CNN "Continuous Current"
	1    1550 1700
	1    0    0    -1  
$EndComp
$Comp
L Q_PMOS_DGS Q?
U 1 1 59D315D8
P 2000 1200
F 0 "Q?" H 2200 1250 50  0000 L CNN
F 1 "Q_PMOS_DGS" H 2200 1150 50  0000 L CNN
F 2 "" H 2200 1300 50  0001 C CNN
F 3 "" H 2000 1200 50  0001 C CNN
F 4 "Diodes Incorporated" H 2000 1200 60  0001 C CNN "Manufacturer"
F 5 "DMP2215L-7" H 2000 1200 60  0001 C CNN "Manufacturer PN"
F 6 "20V" H 2000 1200 60  0001 C CNN "Vds"
F 7 "2.5, 4.5V" H 2000 1200 60  0001 C CNN "Drive Voltage"
F 8 "2.7A" H 2000 1200 60  0001 C CNN "Continuous Current"
	1    2000 1200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 59D55DE5
P 1650 2050
F 0 "#PWR?" H 1650 1800 50  0001 C CNN
F 1 "GND" H 1650 1900 50  0000 C CNN
F 2 "" H 1650 2050 50  0001 C CNN
F 3 "" H 1650 2050 50  0001 C CNN
	1    1650 2050
	1    0    0    -1  
$EndComp
Text GLabel 1200 1700 0    60   Input ~ 0
R_Control_1
$Comp
L R R?
U 1 1 59D55E47
P 1650 1050
F 0 "R?" V 1730 1050 50  0000 C CNN
F 1 "R" V 1650 1050 50  0000 C CNN
F 2 "Resistors_SMD:R_0402" V 1580 1050 50  0001 C CNN
F 3 "" H 1650 1050 50  0001 C CNN
F 4 "TE Connectivity Passive Product" V 1650 1050 60  0001 C CNN "Manufacturer"
F 5 "CRG0402J1K0" V 1650 1050 60  0001 C CNN "Manufacturer PN"
F 6 "1 kOhm" V 1650 1050 60  0001 C CNN "Resistance"
F 7 "1/16 W" V 1650 1050 60  0001 C CNN "Power"
	1    1650 1050
	1    0    0    -1  
$EndComp
Connection ~ 1650 1200
$Comp
L +5V #PWR?
U 1 1 59D55EBA
P 1650 800
F 0 "#PWR?" H 1650 650 50  0001 C CNN
F 1 "+5V" H 1650 940 50  0000 C CNN
F 2 "" H 1650 800 50  0001 C CNN
F 3 "" H 1650 800 50  0001 C CNN
	1    1650 800 
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR?
U 1 1 59D55EDA
P 2100 900
F 0 "#PWR?" H 2100 750 50  0001 C CNN
F 1 "+5V" H 2100 1040 50  0000 C CNN
F 2 "" H 2100 900 50  0001 C CNN
F 3 "" H 2100 900 50  0001 C CNN
	1    2100 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 900  2100 1000
Wire Wire Line
	1650 800  1650 900 
$Comp
L LED_CRGB D?
U 1 1 59D55F2A
P 9550 1100
F 0 "D?" H 9550 1470 50  0000 C CNN
F 1 "LED_CRGB" H 9550 750 50  0000 C CNN
F 2 "" H 9550 1050 50  0001 C CNN
F 3 "" H 9550 1050 50  0001 C CNN
	1    9550 1100
	-1   0    0    1   
$EndComp
Wire Wire Line
	1650 1200 1800 1200
Wire Wire Line
	1650 1200 1650 1500
Wire Wire Line
	1200 1700 1350 1700
Text GLabel 2350 1400 2    60   Input ~ 0
R1
Wire Wire Line
	2350 1400 2100 1400
Wire Wire Line
	1650 1900 1650 2050
$EndSCHEMATC
