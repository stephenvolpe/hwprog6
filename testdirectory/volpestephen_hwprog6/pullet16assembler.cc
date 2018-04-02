#include "pullet16assembler.h"

/******************************************************************************
 *3456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
 * Class 'Assembler' for assembling code.
 *
 * Author: Stephen Volpe
 * Used with permission and modified by: Stephen Volpe
 * Date: 23 November 2017
**/

/******************************************************************************
 * Constructor
**/
//This constructor was given to us by Buell.
Assembler::Assembler() {
  globals_ = Globals();

  mnemonics_.insert("ADD");
  mnemonics_.insert("AND");
  mnemonics_.insert("BAN");
  mnemonics_.insert("BR "); // need the blank space
  mnemonics_.insert("LD "); // need the blank space
  mnemonics_.insert("STC");
  mnemonics_.insert("SUB");
  mnemonics_.insert("RD "); // need the blank space
  mnemonics_.insert("STP");
  mnemonics_.insert("WRT");
//  mnemonics_.insert("DS "); // need the blank space
//  mnemonics_.insert("HEX");
//  mnemonics_.insert("ORG");
//  mnemonics_.insert("END");

  opcodes_["ADD"] = "100x            ";
  opcodes_["AND"] = "011x            ";
  opcodes_["BAN"] = "000x            ";
  opcodes_["BR "] = "110x            ";
  opcodes_["LD "] = "101x            ";
  opcodes_["STC"] = "010x            ";
  opcodes_["SUB"] = "001x            ";
  opcodes_["RD "] = "1110000000000001";
  opcodes_["STP"] = "1110000000000010";
  opcodes_["WRT"] = "1110000000000011";

  for (int linesub = 0; linesub < globals_.kMaxMemory; ++linesub) {
    machinecode_[linesub] = kDummyCodeA;
  }
}

/******************************************************************************
 * Destructor
**/
Assembler::~Assembler() {
}

/******************************************************************************
 * Accessors and Mutators
**/

/******************************************************************************
 * General functions.
**/

/******************************************************************************
 * Function 'Assemble'.
 * This top level function assembles the code.
 *
 * Parameters:
 *   in_scanner - the scanner to read for source code
 *   out_stream - the output stream to write to
**/
void Assembler::Assemble(Scanner& in_scanner, string binary_filename, ofstream& out_stream) {
#ifdef EBUG
  Utils::log_stream << "enter Assemble\n"; 
#endif

  ////////////////////////////////////////////////////////////////////////////
  // Pass one
  // Produce the symbol table and detect errors in symbols.
  Utils::log_stream << endl << endl << "PASS ONE" << endl;
  this->PassOne(in_scanner);
  this->PrintCodeLines();
  this->PrintSymbolTable();

  ////////////////////////////////////////////////////////////////////////////
  // Pass two
  // Generate the machine code.
  Utils::log_stream << endl << endl << "PASS TWO" << endl;
  this->PassTwo();
  
  ///////////////////////////////////////////////////////////////////////////
  //Dump the results
  this->PrintCodeLines();
  this->PrintSymbolTable();
  //Check if it has an error
  //If it DOES NOT have an error, it prints the machine codes
  if (has_an_error_) {

    Utils::log_stream << endl << "ERRORS EXIST" << endl;
    Utils::log_stream << "NO MACHINE CODE GENERATED" << endl;
  } else {
    Utils::log_stream << endl << endl << "MACHINE CODE" << endl;
    this->PrintMachineCode(binary_filename, out_stream);
  }

#ifdef EBUG
  Utils::log_stream << "leave Assemble\n"; 
#endif
}

/******************************************************************************
 * Function 'GetInvalidMessage'.
 * This creates a "value is invalid" error message.
 * This function is formatted to try to recreate Buell's Example code.
 * Parameters:
 *   leadingtext - the text of what it is that is invalid
 *   symbol - the symbol that is invalid
**/
string Assembler::GetInvalidMessage(string leadingtext, string symbol) {

  string returnvalue = "";

  returnvalue += "\n**** ERROR -- ";
  returnvalue += leadingtext + " ";
  returnvalue += symbol + " ";
  returnvalue += "IS INVALID";
 
  return returnvalue;
}

/******************************************************************************
 * Function 'GetInvalidMessage'.
 * This creates a "value is invalid" error message.
 *
 * Parameters:
 *   leadingtext - the text of what it is that is invalid
 *   hex - the hex operand that is invalid
**/
string Assembler::GetInvalidMessage(string leadingtext, Hex hex) {
 
  string returnvalue = "";

  returnvalue += "\n***** ERROR -- ";
  returnvalue += leadingtext + " ";
  returnvalue += hex.GetText() + " ";
  returnvalue += "IS INVALID";
  

  return returnvalue;
}

/******************************************************************************
 * Function 'GetUndefinedMessage'.
 * This creates a "symbol is undefined" error message.
 *
 * Parameters:
 *   badtext - the undefined symbol text
**/
string Assembler::GetUndefinedMessage(string badtext) {

  string returnvalue = "";
  returnvalue += "\n***** ERROR -- SYMBOL ";
  returnvalue += badtext;
  returnvalue += " IS UNDEFINED";

  return returnvalue;
}

/******************************************************************************
 * Function 'PassOne'.
 * Produce the symbol table and detect multiply defined symbols.
 *
 * CAVEAT: We have deliberately forced symbols and mnemonics to have
 *         blank spaces at the end and thus to be all the same length.
 *         Symbols are three characters, possibly with one or two blank at end.
 *         Mnemonics are three characters, possibly with one blank at end.
 *
 * Parameters:
 *   in_scanner - the input stream from which to read
 *   out-stream - the output stream to which to write
**/
void Assembler::PassOne(Scanner& in_scanner) {
#ifdef EBUG
  Utils::log_stream << "enter PassOne\n"; 
#endif
  /*
  * THE FOLLOWING IS FROM SOLUTION 6A FROM BUELL
  */
  pc_in_assembler_ = 0;
  maxpc_ = max(maxpc_, pc_in_assembler_);
  UINT linecounter = -1;
  string line = in_scanner.NextLine();
  while (line.length() > 0) {
    ++linecounter;
    CodeLine codeline = CodeLine(globals_);
    // arguments are:
    // linecounter, pc, label, mnemonic, addr, symoper, hexoper,
    //     comments, machinecode
    codeline.SetCodeLine(0, 0, "nulllabel", "nullmnemonic", 
                         "nulladdr", "nullsymoperand", "nullhexoperand", 
                         "nullcomments", kDummyCodeA);
    if (line.at(0) == '*') {
      codeline.SetCommentsOnly(linecounter, line);
      codelines_.push_back(codeline);
    } else {
      
      string label = codeline.GetLabel();
      if (line.length() >= 3) {
        label = (line + "   ").substr(0, 3);
      }

      string mnemonic = codeline.GetMnemonic();
      if (line.length() >= 5) {
        mnemonic = (line + "   ").substr(4, 3);
      }
      if (mnemonic == "END") {
        found_end_statement_ = true;
      }

      string addr = codeline.GetAddr();
      if (line.length() >= 9) {
        addr = (line + "   ").substr(8, 1);
      }

      string symoper = codeline.GetSymOperand();
      if (line.length() >= 11) {
        symoper = (line + "   ").substr(10, 3);
      }

      string hexoper = codeline.GetHexObject().GetText();
      if (line.length() >= 15) {
        hexoper = (line + "   ").substr(14, 5);
      }

      string comments = codeline.GetComments();
      if (line.length() >= 21) {
        comments = line.substr(20);
      } 
      codeline.SetCodeLine(linecounter, pc_in_assembler_, label, mnemonic, 
                           addr, symoper, hexoper, comments, kDummyCodeA);

      this->UpdateSymbolTable(pc_in_assembler_, codeline.GetLabel());
      this->SetNewPC(codeline);
      codelines_.push_back(codeline);
    }
    line = in_scanner.NextLine();
  } // while (line.length() > 0) {

#ifdef EBUG
  Utils::log_stream << "leave PassOne\n"; 
#endif
}
/******************************************************************************
 * Function 'PassTwo'.
 * This function does pass two of the assembly process.
 * Error checking is very repetitive
 * I tried to be extremely systematic and check for errors given from
 * Snippets of the given homework exercise question
**/
void Assembler::PassTwo() {
#ifdef EBUG
  Utils::log_stream << "enter PassTwo\n"; 
#endif
  //INIT
  found_end_statement_ = false;
  pc_in_assembler_ = 0;
  maxpc_ = max(maxpc_, pc_in_assembler_);

  for (auto iter = codelines_.begin(); iter != codelines_.end(); ++iter) {
    string errormessage = "";

    //if entire line is a comment, don't care and keep going (continue)
    if ((*iter).IsAllComment()) {
      continue;
    }
    //Now the fun begins because it isnt a simple comment...
    
    //LOOK OUT FOR THE GOTCHA
    //Design was talked about in SI and boy oh boy did that make this a lot
    //Easier to understand
    //PC is set to zero at the top and then calculated at the bottom
    //of this loop
    (*iter).SetPC(pc_in_assembler_);
    maxpc_ = max(maxpc_, pc_in_assembler_);

    //we have to check for labels

    //check for invalid labels
    if ((*iter).HasLabel()) {
      string label = (*iter).GetLabel();

      // label is not in our table
      if (symboltable_.count(label) == 0) {
        has_an_error_ = true;
        errormessage += this->GetUndefinedMessage(label);
      }

      //label is not a valid label
      else if (symboltable_.at(label).GetErrorMessages().length() > 0) {
        has_an_error_ = true;
        errormessage += "\n" + symboltable_.at(label).GetErrorMessages();
      }
    }
    
    //check for invalid sym opeerands
    //Set the symbol hex to a dummyCode that we can track
    //Design idea taken from SI
    string symbolhex = kDummyCodeB;
    if ((*iter).HasSymOperand()) {

      string operand = (*iter).GetSymOperand();

      //if the operand does not exist in our symbol table
      if (symboltable_.count(operand) == 0) {
        has_an_error_ = true;
        errormessage += this->GetUndefinedMessage(operand);
        //set a bogus value to be trackable - idea taken from SI
        //symbolhex == 111100001111
        symbolhex = kDummyCodeB;
      }
      //sym exists with an operand
      else {
        Symbol symbol = symboltable_.at(operand);
        errormessage += symbol.GetErrorMessages();
        has_an_error_ = has_an_error_ || symbol.HasAnError();
        int location = symbol.GetLocation();
        symbolhex = globals_.DecToBitString(location, 12);
      }
    }
    
    //2) Mnemonincs - I HATE THIS WORD AND CANT TYPE IT WORTH A CRAP, i used nem
    //Now we get down to dealing with the mnemonics
    string nem = (*iter).GetMnemonic();
    
    //If the nem exists in our opcodes_
    if (opcodes_.count(nem) > 0) {
      string code = opcodes_.at(nem);
      code = code.substr(0,3) + (*iter).GetAddr() + code.substr(4); //style idea from SI
      (*iter).SetMachineCode(code);

      //set machine code if it has a symbolhex operand
      //add symbolhex to end
      if ((*iter).HasSymOperand()) {
        code = code.substr(0, 4) + symbolhex;
        (*iter).SetMachineCode(code);
      }
    }

    //if the mnenomic is "HEX"
    else if (nem == "HEX") {
      if ((*iter).GetHexObject().HasAnError()) {
        has_an_error_ = true;
        errormessage += (*iter).GetHexObject().GetErrorMessages();
      }
      
    //the hex object is valid
    //set machine code
      else {
        int hex = (*iter).GetHexObject().GetValue();
        string code = globals_.DecToBitString(hex, 16);
        (*iter).SetMachineCode(code);
      }
    }

    //ORG
    else if (nem == "ORG") {
      if ((*iter).GetHexObject().HasAnError()) {
        has_an_error_ = true;
        errormessage += (*iter).GetHexObject().GetErrorMessages();
      } else {
        int hex = (*iter).GetHexObject().GetValue();
 
        //Check for out of bounds mem address
        if ((hex >= globals_.kMaxMemory) || (hex < 0)) {
          has_an_error_ = true;
          errormessage += this->GetInvalidMessage("PC VALUE",
            (*iter).GetHexObject());
        }
      }
    }
    else if (nem == "DS ") { //
      if ((*iter).GetHexObject().HasAnError()) {
        has_an_error_ = true;
        errormessage += (*iter).GetHexObject().GetErrorMessages();
      }
   //Check for memory out of bounds
   else {
     int hex = (*iter).GetHexObject().GetValue();
     if ((hex > globals_.kMaxMemory) || (hex <= 0)) {
       has_an_error_ = true;
       errormessage += this->GetInvalidMessage("DS ALLOCATION", 
       (*iter).GetHexObject());
     }//if not out of bounds, set to DUmmyC
     else {
       (*iter).SetMachineCode(kDummyCodeC);//Once again - style idea taken from SI
       }
     }
   }


   //END - easy enough, we found an end statement, hooray.
   else if (nem == "END") {
     found_end_statement_ = true;
     //DummyCodeD == 0000000011110000 style idea taken from SI
     (*iter).SetMachineCode(kDummyCodeD);
    }

  //Invalid Nem
    else {
      has_an_error_ = true;
      errormessage += this->GetInvalidMessage("MNENONIC", nem);
    }

    //check if we had errors
    if(errormessage.length() > 0) {
      has_an_error_ = true;
      errormessage += "\n";
      (*iter).SetErrorMessages(errormessage);
    }
    
    //set the code to the machine code map using keyword - ->second, thank you Cplusplus.com
    //This was such a pain in my rear
    machinecode_[pc_in_assembler_] = (*iter).GetCode();
    this->SetNewPC(*iter);
    maxpc_ = max(maxpc_, pc_in_assembler_);

  }

#ifdef EBUG
  Utils::log_stream << "leave PassTwo\n";
#endif
}

/******************************************************************************
 * Function 'PrintCodeLines'.
 * This function prints the code lines.
 * This function was given to us in solution
**/
void Assembler::PrintCodeLines() {
#ifdef EBUG
  Utils::log_stream << "enter PrintCodeLines\n"; 
#endif
  string s = "";//

  for (auto iter = codelines_.begin(); iter != codelines_.end(); ++iter) {
    s += (*iter).ToString() + '\n';
  }

  if (!found_end_statement_) {
    s += "\n***** ERROR -- NO 'END' STATEMENT\n";
    has_an_error_ = true;
  }

#ifdef EBUG
  Utils::log_stream << "leave PrintCodeLines\n"; 
#endif
  Utils::log_stream << s << endl;
}

/*******************************************************************************
 * Function 'PrintMachineCode'.
 * This function prints the machine code.
 * I owe a lot of this to SI and Cplusplus.com snippets
 * I over documented this in my opinion because I wrote it all out and filled in 
 * What I needed to do with the code as I looked it up online.
**/
void Assembler::PrintMachineCode(string binary_filename, ofstream& out_stream) {
  #ifdef EBUG
  #endif
  string s = "";
  Utils::log_stream << "end PrintMachineCode" << " " << binary_filename << 
  endl;
  FILE *fp = fopen(binary_filename.c_str(), "w");
  //this formatting was discussed in SI in order to match buell's output.
  for (int i = 0; i < maxpc_; ++i) {
    s += Utils::Format(i, 4);
    s += " " + globals_.DecToBitString(i, 12);
    string tempcode = machinecode_.at(i);
    s += " " + tempcode.substr(0, 4);
    s += " " + tempcode.substr(4, 4);
    s += " " + tempcode.substr(8, 4);
    s += " " + tempcode.substr(12, 4);
    s += "\n";
    
    out_stream << tempcode << endl;
    //The following was help from SI
    //make a vint value of the code
    int bin = globals_.BitStringToDec(tempcode);
    //"shift" or convert however, you want to word it
    int bin1 = bin & 255;
    char char1 = static_cast<char>(bin1);
    
    int bin2 = (bin>>8) & 255;
    char char2 = static_cast<char>(bin2);
    //Write to stream
    //Thanks cplusplus.com
    //AS SAID IN CLASS NUMEROUS TIMES
    //THE BYTES ARE REVERSED
    //so we have to fix it
    putc(char2, fp);
    putc(char1, fp);
  }
  fclose(fp);

  Utils::log_stream << s << endl;
  
  vector<char> thebytes;
  char onebyte;
  fp = fopen(binary_filename.c_str(), "r");
  //find end of file
  //so we can run a loop
  fseek(fp, 0 , SEEK_END);
  int filesize = ftell(fp);
  fclose(fp);

  //populate the character vector
  fp = fopen(binary_filename.c_str(), "r");
  for (int i = 0; i < filesize; ++i) {
    onebyte = getc(fp);
    thebytes.push_back(onebyte);
  }
  fclose(fp);

  for (int i = 0; i < thebytes.size(); i += 2) { //Help from SI on this section.
    //shift
    int top = thebytes.at(i) && 255;
    int bot = thebytes.at(i+1) & 255;
    //short = 16 bit, perfect.
    short binvalue = top*256 + bot;

    string realcode = globals_.DecToBitString(binvalue, 16);
    //Check if what we computed is 
    //Equal to the machine code
    //If not, then we messed up, and exit
    if (realcode != machinecode_.at(i/2)) { //I repeat again, if it does not match the machinecodes_ exit, we messed up.
      //formatting help from SI to match buell's output
      cout << ("ERROR INBINARY %8d X%sX X%sX\n", binvalue, realcode.c_str(),
                                                 machinecode_.at(i/2).c_str());
      exit(0);
    }
  }

#ifdef EBUG
  Utils::log_stream << "leave PrintMachineCode" << endl;
#endif
}

/**********************************************************************************
 * Function 'PrintSymbolTable'
 * This function prints the symbol table
*/
void Assembler::PrintSymbolTable() {
#ifdef EBUG
  Utils::log_stream << "enter PrintSymbolTable" << endl;
#endif
  string s = "SYMBOL TABLE\n    SYM LOC FLAGS \n";
  //iterate though the symbol table with some formatting
  for (auto iter = symboltable_.begin(); iter != symboltable_.end(); ++iter) {
    Symbol symbol = iter->second;
    s += "SYM " + symbol.ToString() + "\n";
  }

#ifdef EBUG
  Utils::log_stream << "leave PrintSymbolTable\n";
#endif
  Utils::log_stream << s << endl;
}

/*********************************************************************************
 * Function 'SetNewPC'
 *This function gets a new value for the program counter.
 *
 * No return value - this sets the class's PC variable.
 *
 * Parameters:
 * codeline - the line of code from which to update
*/
void Assembler::SetNewPC(CodeLine codeline) {
#ifdef EBUG
  Utils::log_stream << "enter SetNewPC\n";
#endif
  //THIS IS EXTREMELY REDUNDANT
  //Advised through pseduocode to check
  //every possible nem again
  //I had the hardest time with this
  int newPC = pc_in_assembler_;
  string nem = codeline.GetMnemonic();

  if (codeline.IsAllComment()) {//no increase
    newPC = pc_in_assembler_;
  } else if (nem.length() == 0) {//no increase
    newPC = pc_in_assembler_;
  } else if (nem == "   ") {//no increase
    newPC = pc_in_assembler_;
  } else if (nem == "END") {//no increase
    newPC = pc_in_assembler_;
  } else if (nem == "DS ") {//Must have white space after DS
    if(codeline.GetHexObject().HasAnError()) {//has an error = no increase
      newPC = pc_in_assembler_;
    } else {
      int increase = codeline.GetHexObject().GetValue();
      if ((increase > globals_.kMaxMemory) || (increase <= 0)) {//out of bounds - no increase
        newPC = pc_in_assembler_;
      } else {//in bounds, do increase.
        newPC = pc_in_assembler_ + increase;
      }
    }
  } else if (nem == "ORG") {
    if (codeline.GetHexObject().HasAnError()) {
      newPC = pc_in_assembler_;
    } else {
      int value = codeline.GetHexObject().GetValue();
      if ((value >= globals_.kMaxMemory) || (value < 0)) {//out of bounds, no increase
        newPC = pc_in_assembler_;
      } else { //increase its valid
        newPC = value;
      }
    }
  } else if (mnemonics_.count(nem) > 0) {//valid increase
    newPC = pc_in_assembler_ + 1;
  } else { //valid increase
    newPC = pc_in_assembler_ + 1;
  }

  pc_in_assembler_ = newPC;
  maxpc_ = max(maxpc_, pc_in_assembler_); //recalculate max

#ifdef EBUG
  Utils::log_stream << "leave SetNewPC\n";
#endif
}

/************************************************************************************
 *Function 'UpdateSymbolTable'
 *This function updates the symbol table for a putative symbol.
 * Note that there si a hack here, in that the default value iso 0
 * and that would mean we cant store a symbol at location zero
 *so we add one, and then back that out after the full first pass is done
 *
 *returns
 *the pretty pring string for printing
 **/
void Assembler::UpdateSymbolTable(int pc, string symboltext) {
#ifdef EBUG
  Utils::log_stream << "enter UpdateSymbolTable\n";
#endif

  //If the symbol is not null or "   " (same thing)
  if ((symboltext == "nulllabel") || (symboltext == "   ")) {
    return;
  }
  
  //check if key is there
  if (symboltable_.count(symboltext) > 0) {//Key is not there
    //copy symbol
    //flag multiple:
    //reassign
    Symbol oldsymbol = symboltable_[symboltext];
    oldsymbol.SetMultiply();
    //Core dumped if i used .at on the symboltable
    //troubleshot in SI
    symboltable_[symboltext] = oldsymbol;
  } else {
    Symbol symbol = Symbol(symboltext, pc_in_assembler_);
    symboltable_[symboltext] = symbol;
  }

#ifdef EBUG
  Utils::log_stream << "leave UpdateSymbolTable\n";
#endif
}
