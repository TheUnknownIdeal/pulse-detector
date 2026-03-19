#ifndef PROCESSING_H
#define PROCESSING_H


class SignalStream {
    public:
        SignalStream(); // Contructor

        // main operation
        int64_t rolling(uint32_t& ir, uint32_t& red, uint16_t dp, bool& hb, int32_t& slope);


        // Time stamp for previous heartbeat

        // Minimum samples between heartbeats
        static constexpr uint16_t  hb_delay = 20; // At 100 Hz, this is 200 ms

        uint16_t t; // Period measured in samples

        float spo2;
       


    private:

        // Width of the AC bucket
        static constexpr uint16_t _wAC = 20;
        uint32_t _ir;
        uint32_t _red;

        // Add raw sample
        void _updateP(uint32_t& ir, uint32_t& red);


        
        // Registers for detecting heartbeat
        static constexpr uint16_t _w = 15; // The width of a delta (number of samples)
        uint32_t _hb[_w]; // Processed IR samples
        int32_t _last_delta; // The previous computed delta

        // Delta = current Average - past average
        int32_t _write_new_delta(int32_t new_val, bool& hb);

        // _c1 is counting (in samples) the duration of the current period
        static constexpr uint16_t _wt = 5; // How many ts the period bucket will store ("w" stands for width)
        uint16_t _c1;

        // DC Buckets
        uint32_t _irDC;
        uint32_t _redDC;

        void _spo2_compute();

        void _get_DC(uint32_t& ir, uint32_t& red);

        







};

#endif