#ifndef MV_NAME_IT_HPP__
#define MV_NAME_IT_HPP__

// The name iterator is an efficient way to increment the name string
// While it would be easier to just use the spritntf function
//  Because this is so specific, this will be more efficient - therefore reducing battery & time taken
//      (with embedded stuff you should be super specific)



#include "globals.h"

namespace mv{
class name_it {
    static char name[26];
    static constexpr int num_numbers = 5;
    uint8_t num_begin, num_end;
    MV_ALWAYS_INLINE void increment_place(){
        uint8_t conv = num_end;
        for(int i = 0; i < num_numbers; ++i, --conv){
            name[conv] = (name[conv] == '9' ? '0' : char(name[conv] + 1));
            if(name[conv] != '0') return;
        }
    }
    public:

        name_it()
            :num_begin(16), num_end(20)
        {}

        MV_ALWAYS_INLINE name_it& operator++(){
            increment_place();
            return *this;
        }

        MV_ALWAYS_INLINE const char* c_str() const noexcept { return this->name; }
};

// 25 character string
char name_it::name[26] = "/sdcard/video/v_00000.raw"; 

}

#endif // MV_NAME_IT_HPP__
