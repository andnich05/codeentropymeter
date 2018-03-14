/*
 * PeakMeter: Calculation of peak values
 *
 * Copyright (C) 2014  Andrej Nichelmann
 *                     Klaus Michael Indlekofer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PeakMeter.hpp"
#include <math.h>
#include <QDebug>

const double INF = -999;

PeakMeter::PeakMeter(PeakMeterListener *listener)
    : peakMeterListener(listener)
    , actualValue(-60)
    , returnTimeValue(0)
    , currentValue(0)
    , referenceValue(0)
    , maxValue(0)
    , absoluteValue(0)
    , maximumDynamicRange(0)
{
}

void PeakMeter::updateMeter(const std::vector<int32_t> & signalValues) {
    if(!peakMeterListener)
    {
        return;
    }

    // Get the maximum value of the samples
    currentValue = getMaximum(signalValues);
    emitPeakValue(calculatePeak(currentValue, referenceValue));
}

void PeakMeter::updateBitdepth(int bitdepth) {
    referenceValue = pow(2, bitdepth-1);
    maximumDynamicRange = 20*log10(pow(2,bitdepth)/(double)2);
}

quint32 PeakMeter::getMaximum(const std::vector<int32_t> & signalValues) {
    maxValue = 0;
    absoluteValue = 0;
    for(const auto& signalValue : signalValues) {
        absoluteValue = std::abs(signalValue);
        maxValue = std::max(maxValue, absoluteValue);
    }
    return maxValue;
}

double PeakMeter::calculatePeak(int currentValue, int referenceValue) {
    if(currentValue > 0) {
        return 20*log10((double)currentValue/(double)referenceValue);
    }
    else {
        return INF;
    }
}

void PeakMeter::emitPeakValue(double peak) {
    // Check if the calculated value is greater than the value which is currently being displayed
    if(peak > actualValue) {
        actualValue = peak;
        // Check if the holder needs to be updated
        if(peak >= -60) {
            peakMeterListener->receivePeakHolderValue(actualValue);
        }
    }
    // Check if the signal is clipping
    else if(peak >= 0) {
        actualValue = 0;
    }

    // Update current meter value
    peakMeterListener->receivePeakMeterValue(actualValue);

    // Decrement current value if the calculated value is smaller (return time)
    if(peak < actualValue) {
        if(actualValue > -maximumDynamicRange) {
            actualValue -= returnTimeValue;
        }
        else {
            actualValue = INF;
        }
    }
}

void PeakMeter::setReturnTimeValue(double value) {
    returnTimeValue = value;
}
