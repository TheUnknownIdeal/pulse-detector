#ifndef PROCESSING_H
#define PROCESSING_H


class SignalStream {
    public:
        SignalStream();

        // main operation
        void update(uint32_t& ir, uint32_t& red);

        // Print for debugging
        void print_stream();


    private:

        // An ir value below this threshold is considered a bad sample (no finger)
        static const uint32_t _threshold = 50000;

        // Number of samples held
        static const uint16_t _len = 300;

        uint32_t _ir[_len];
        uint32_t _red[_len];

        // index to iterrate through array
        uint16_t _i;

        // keeps track of how mnay array elements have been filled since beginninf of run
        uint16_t _filled;

        void _add_value(uint32_t& ir, uint32_t& red);

        
 

        //void _compute_
    

};

#endif