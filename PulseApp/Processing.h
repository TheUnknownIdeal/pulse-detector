#ifndef PROCESSING_H
#define PROCESSING_H


class SignalStream {
    public:
        SignalStream(); // Contructor

        // main operation
        uint64_t update(uint32_t& ir, uint32_t& red, bool& hb, int32_t& slope);


        // Time stamp for previous heartbeat

        // Minimum samples between heartbeats
        static constexpr uint16_t  hb_delay = 20; // At 100 Hz, this is 200 ms


        void getP(uint32_t& ir,uint32_t& red);
        void getDC(uint32_t& ir,uint32_t& red);

        void getT(uint16_t& t);

        void getSpO2(float& SpO2);
       


    private:

        // Buckets for IR and RED values
        uint32_t _bIR;
        uint32_t _bRED;
        static constexpr uint16_t _wAC = 20;

        // DC Buckets
        uint32_t _bIR_DC; // They have a width of t (period)
        uint32_t _bRED_DC;
        static constexpr uint16_t _wDC = 200;


        uint32_t _bt; // Period bucket
        static constexpr uint16_t _wt = 5; // How many ts the period bucket will store ("w" stands for width)
        uint16_t _ct; // counter for counting period (in samples)
        

        // Registers for detecting heartbeat
        static constexpr uint16_t _w = 15; // The width of a delta (number of samples)
        uint32_t _hb[_w]; // Processed IR samples
        int32_t _last_delta; // The previous computed delta

        // Delta = current Average - past average
        int32_t _detect_HB(int32_t new_val, bool& hb);

        long _bSpO2;
        static constexpr uint16_t _wSpO2 = 3;

        void _update_SpO2();
        
        // min and max values for calculating amplitudes for SpO2
        uint32_t _minIR, _maxIR;
        uint32_t _minRED, _maxRED;

};

#endif