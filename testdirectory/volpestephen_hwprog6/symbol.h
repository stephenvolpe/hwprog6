/****************************************************************
 * Header file for the 'Symbol' class to contain one symbol.
 *
 * Author/copyright:  Stephen Volpe
 * Used with permission and modified by: Stephen Volpe
 * Date: 23 November 2017
 *
**/

#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
using namespace std;

//#include "../../Utilities/scanner.h"
//#include "../../Utilities/scanline.h"
#include "../../Utilities/utils.h"

class Symbol {
  public:
    Symbol();
    Symbol(string symboltext, int programcounter);
    virtual ~Symbol();

    string GetErrorMessages() const;
    int GetLocation() const;
    bool HasAnError() const;
    void SetMultiply();
    string ToString() const;

  private:
    int location_;
    bool is_multiply_;
    bool is_invalid_;
    string error_messages_;
    string text_;

    bool CheckInvalid() const;
};

#endif
