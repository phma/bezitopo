/******************************************************/
/*                                                    */
/* measure.h - measuring units                        */
/*                                                    */
/******************************************************/

#ifndef MEASURE_H
#define MEASURE_H
/* These are unit codes. Codes pertaining to angles are defined in angle.h. */
#define METER 0x00011000
#define MILLIMETER 0x00010d00
#define MICROMETER 0x00010c00
#define KILOMETER 0x00011300
#define MEGAMETER 0x00011400
#define INCH 0x0001ec00
#define FOOT 0x0001ea00
#define SURVEYFOOT 0x0001f000
#define CHAIN 0x0001f100
#define SURVEYCHAIN 0x0001f200
#define MIL 0x0001e500
#define GRAM 0x00021000
#define KILOGRAM 0x00021300
#define POUND 0x0002ed00
#define MILLIPOUND 0x0002f000
#define OUNCE 0x0002ee00
#define TROYOUNCE 0x0002e900
#define HOUR 0x0003ee00
#define KGPERL 0x00801400
#define LBPERIN3 0x0080ef00
#define PERKG 0x00291300
#define PERPOUND 0x0029ed00
#define PERMETER 0x002a1000
#define PERFOOT 0x002aeb00
#define PERLOT 0x40000000
#define ITEM 0x04001000
#define THOUSAND 0x04001300
#define EACH 0x04011000
#define PERHUNDRED 0x04011200
#define PERTHOUSAND 0x04011300
#define PERSET 0x40010000
#define LOT 0x40020000
#define PERLITER 0x00353000
#define SET 0x40030000
#define PERMONTH 0x0036eb00
#define PERYEAR 0x0036ea00
#define PERHOUR 0x0036ee00
#define MILLILITER 0x00282d00
#define IN3 0x0028ec00
/* These convert as different units depending on the selected measuring system. */
#define MASS1 0x0002fe00
#define MASS2 0x0002fd00
#define LENGTH1 0x0001fe00
#define LENGTH2 0x0001fd00
#define ANGLE 0x00380000
/* These are precision codes. DEC3 = 3 digits after the decimal point. */
#define DEC0 0x00
#define DEC1 0x01
#define DEC2 0x02
#define DEC3 0x03
#define DEC4 0x04
#define DEC5 0x05
#define DEC6 0x06
#define HALF 0x11
#define QUARTER 0x12
#define EIGHTH 0x13
#define SIXTEENTH 0x14
#define THIRTYSECOND 0x15

#define compatible_units(unitp1,unitp2) (((unitp1)&0xffff0000)==((unitp2)&0xffff0000))
#define same_unit(unitp1,unitp2) (((unitp1)&0xffffff00)==((unitp2)&0xffffff00))

#define badunits 4
// error thrown by conversion routine

int is_exact(double measurement, unsigned int unitp,int is_toler);
/* Checks whether the given measurement is exact in that unit.
   If the measurement is a tolerance and is divisible by 127, returns false;
   this means that a tolerance in inches is being checked in millimeters. */
char *format_meas(double measurement, unsigned int unitp);
char *format_meas_unit(double measurement, unsigned int unitp);
double parse_meas(const char *meas, int unitp,int *found_unit);
/* Given a string representing a measurement, in the unit unitp unless specified otherwise,
   returns its value in the program's internal unit. */
char *trim(char *str);
/* Removes spaces from both ends of a string in place. */
void switch_system();
/* Switches between measuring systems. */
void set_length_unit(int unitp);
double precision(int unitp);
double parse_length(const char *meas);
#endif
