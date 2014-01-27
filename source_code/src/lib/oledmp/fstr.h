#ifndef FSTR_H_
#define FSTR_H_

// type for flash strings (progmem strings)
class __fstr;
#define F(string) (reinterpret_cast<const __fstr *>(PSTR(string)))

#endif
