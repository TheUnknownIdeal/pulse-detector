#include "Processing.h"


container::container() {
    _i = 0;
    _filled = 0;
}

container::_add_value(uint32_t& ir, uint32_t& red) {

    _ir[++_i] = ir;
    _red[_i] = red;

    if (_filled < _len) _filled++;

}
