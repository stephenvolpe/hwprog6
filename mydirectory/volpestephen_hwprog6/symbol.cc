#include "symbol.h"

/******************************************************************************
 *3456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
 * Class 'Symbol' as a container for one symbol.
 * This class was written by the author below
 * Author: Duncan A. Buell
 * Used with permission and modified by: Stephen Volpe
 * Date: 23 November 2017
**/

/******************************************************************************
 * Constructor
**/
Symbol::Symbol() {
}

/******************************************************************************
 * Constructor
**/
Symbol::Symbol(string text, int programcounter) {
  location_ = programcounter;
  is_multiply_ = false;
  text_ = text;
  is_invalid_ = this->CheckInvalid();
}

/******************************************************************************
 * Destructor
**/
Symbol::~Symbol() {
}

/******************************************************************************
 * Accessors and Mutators
**/

/******************************************************************************
 * Accessor for 'error_messages_'.
 * There is a kluge here for getting the internal newline character.
**/
string Symbol::GetErrorMessages() const {
  bool previouserror = false;
  string error_messages = "";

  if (is_invalid_) {
    error_messages += "***** ERROR -- SYMBOL " + text_ + " IS INVALID";
    previouserror = true;
  }
  if (is_multiply_) {
    if (previouserror) {
      error_messages += "\n";
    }
    error_messages += "***** ERROR -- SYMBOL " + text_ + " IS MULTIPLY DEFINED";
    previouserror = true; // set this just in case we add more cases later
  }

  return error_messages;
}

/******************************************************************************
 * Accessor for the 'location_'.
**/
int Symbol::GetLocation() const {
  return location_;
}

/******************************************************************************
 * Accessor for the existence of errors.
**/
bool Symbol::HasAnError() const {
  return (is_invalid_ || is_multiply_);
}

/******************************************************************************
 * Mutator 'SetMultiply'.
 * Sets the 'is_multiply' value to 'true'.
**/
void Symbol::SetMultiply() {
  is_multiply_ = true;
}

/******************************************************************************
 * General functions.
**/

/******************************************************************************
 * Function 'CheckInvalid'.
 * Returns the boolean to say whether a symbol is invalid.
**/
bool Symbol::CheckInvalid() const {
  bool returnvalue = false; // false means no, not invalid

  if (text_ == "   ") {
    return false; // not invalid
  }

  // This is admittedly a kluge.
  // At this point, the symbol is not all blanks.
  char char0 = text_.at(0);
  char char1 = text_.at(1);
  char char2 = text_.at(2);

  if (char0 == ' ') {
    if ((char1 != ' ') || (char2 != ' ')) {
      // starts with blank, but nonblanks follow--WRONG!
      returnvalue = true; // false means no, not invalid
      return returnvalue; // yes invalid
    } else {
      returnvalue = false; // false means no, not invalid
      return returnvalue; // no, not invalid
    }
  }

  // At this point, not all blanks, and not leading blank.
  // Verify we don't have a middle blank but then a nonblank.
  if (char1 == ' ') {
    if (char1 != ' ') {
      returnvalue = true; // false means no, not invalid
      return returnvalue; // yes invalid
    }
  }

  // If we get here, then the symbol is not all blank
  // and if there are blanks they are all on the RHS.

  // Check that the lead character (not a blank) is alpha. 
  if (!isalpha(char0)) {
    returnvalue = true; // false means no, not invalid
    return returnvalue; // yes invalid
  }

  // Check that the next character is blank or is alphanum. 
  if ((char1 != ' ') && (!isalnum(char1))) {
    returnvalue = true; // false means no, not invalid
    return returnvalue; // yes invalid
  }

  // Check that the last character is blank or is alphanum. 
  if ((char2 != ' ') && (!isalnum(char2))) {
    returnvalue = true; // false means no, not invalid
    return returnvalue; // yes invalid
  }

  return returnvalue;
}

/******************************************************************************
 * Function 'ToString'.
 * This function formats an 'Symbol' for prettyprinting.
 *
 * Returns:
 *   the prettyprint string for printing
**/
string Symbol::ToString() const {
#ifdef EBUG
  Utils::log_stream << "enter ToString\n"; 
#endif
  string s = "";

  if (text_ == "nullsymbol") {
    s += Utils::Format("sss", 3);
  } else {
    s += Utils::Format(text_, 3);
  }

  s += Utils::Format(location_, 4);
  if (is_invalid_) {
    s += " INVALID";
  }
  if (is_multiply_) {
    s += " MULTIPLY";
  }

#ifdef EBUG
  Utils::log_stream << "leave ToString\n"; 
#endif
  return s;
}
