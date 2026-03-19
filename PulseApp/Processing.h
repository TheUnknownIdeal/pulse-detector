#ifndef PROCESSING_H
#define PROCESSING_H


class SignalStream {
    public:
        SignalStream(long& hbDelay);

        // main operation
        int64_t update(uint32_t& ir, uint32_t& red, uint16_t dp, bool& hb, long& current_time, int32_t& slope);

        int64_t rolling(uint32_t& ir, uint32_t& red, uint16_t dp, bool& hb, long& current_time, int32_t& slope);

        // Print for debugging
        void print_stream();

        // Time stamp for previous heartbeat
        long hb_stamp;
        long hb_delay; // Minimum time between heartbeats

        


    private:

        // An ir value below this threshold is considered a bad sample (no finger)
        static constexpr uint32_t _threshold = 50000;


        // Number of samples held
        static constexpr uint16_t _len = 150;


        // Space between heartbeats
        static constexpr uint16_t _n = 5;

        // Registers for detecting heartbeat
        uint32_t _hb[_n];
        int32_t _last_slope;

        int32_t _write_new_slope(int32_t new_val, bool& hb, long& timestamp);





        

        uint32_t _ir[_len];
        uint32_t _red[_len];

        // index to iterrate through array
        uint16_t _i;

        void _add_value(uint32_t& ir, uint32_t& red);
        void _getAvg(uint32_t& irAvg, uint32_t& redAvg, uint16_t dp);




};

#endif