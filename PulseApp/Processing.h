#ifndef PROCESSING_H
#define PROCESSING_H


class SignalStream {
    public:
        SignalStream(); // Contructor

        // main operation
        int64_t rolling(uint32_t& ir, uint32_t& red, uint16_t dp, bool& hb, int32_t& slope);

        // Print for debugging
        void print_stream();

        // Time stamp for previous heartbeat

        // Minimum samples between heartbeats
        static constexpr uint16_t  hb_delay = 20; // At 100 Hz, this is 200 ms

        uint16_t period; // Period measured in samples
       


    private:

        // Raw samples
        static constexpr uint16_t _len = 20;
        uint32_t _ir[_len];
        uint32_t _red[_len];
        
        uint16_t _i; // index to iterrate through array

        // Add raw sample
        void _add_value(uint32_t& ir, uint32_t& red);

        // compute averages from raw samples
        void _getAvg(uint32_t& irAvg, uint32_t& redAvg, uint16_t dp);


        
        // Registers for detecting heartbeat
        static constexpr uint16_t _w = 15; // The width of a delta (number of samples)
        uint32_t _hb[_w]; // Processed IR samples
        int32_t _last_delta; // The previous computed delta

        // Delta = current Average - past average
        int32_t _write_new_delta(int32_t new_val, bool& hb);

        // Period data
        static constexpr uint8_t _t = 4;
        uint16_t _periods[_t];
        uint32_t _running_period_sum;

        // _c1 is counting (in samples) the duration of the current period
        uint16_t _c1; // a basic counter for computing periods

        void _get_period();

        // DC Buckets
        uint32_t _irDC;
        uint32_t _redDC;

        void _get_DC(uint32_t& ir, uint32_t& red);

        







};

#endif