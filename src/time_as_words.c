#include "time_as_words.h"

static const char* const HOURS[] = {
  "",
	"satu",
	"dua",
	"tiga",
	"empat",
	"lima",
	"enam",
	"tujuh",
	"dlapan",
	"sembln",
  "sepulh",
  "seblas",
  "duabls"
};

static const char* const ONES[] = {
  "",
	"satu",
	"dua",
	"tiga",
	"empat",
	"lima",
	"enam",
	"tujuh",
	"delapan",
	"sembiln"
};

void time_as_words(int int_hour, int int_min, char* str_hour, char* str_tens, char* str_ones) {
	
	strcpy(str_hour, "");
	strcpy(str_tens, "");
	strcpy(str_ones, "");
  
	//hour
  if (int_hour % 12 == 0) {
		strcat(str_hour, HOURS[12]);
	}
	else {
		strcat(str_hour, HOURS[int_hour % 12]);
	}
	
	//minute
  if(int_min == 0){
    strcat(str_tens, "pas");
  }
  else if (int_min <= 10){
    if(int_min == 10){
      strcat(str_tens, "sepuluh");
    }
    else{
      strcat(str_tens, ONES[int_min % 10]);
    }
  }
	else if (int_min > 10 && int_min < 20) {
    if(int_min == 11){
      strcat(str_tens, "sebelas");
    }
    else {
      strcat(str_tens, ONES[int_min - 10]);
      strcat(str_ones, "belas");
    }	  
	}
	else {    
		strcat(str_tens, ONES[int_min / 10]);
    if(int_min % 10 == 0){
      strcat(str_ones, "puluh");
    }
    else {
      strcat(str_ones, ONES[int_min % 10]);
    }
	}
}