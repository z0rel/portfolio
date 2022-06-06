import React from 'react';
import { CustomDateRangePicker } from '../../index';

export const CustomDateFilterDropdown = ({ onChange, defaultValue = null }) => {
    const DROPDOWN_TOP_ALIGN = {
        points: ['bl', 'tl'],
        offset: [-125, -4],
        overflow: {
            adjustX: 0,
            adjustY: 1,
        },
    };

    return (
        <>
            <div className="custom-filter-dropdown">
                <div className="date-wrapper">
                    <div>
                        <CustomDateRangePicker
                            placeholder={['С', 'По']}
                            format="DD.MM.YYYY"
                            dropdownAlign={DROPDOWN_TOP_ALIGN}
                            defaultValue={defaultValue}
                            onChange={(date) => {
                                let startDate = null;
                                let endDate = null;

                                if(date) {
                                    if(date[0]) {
                                        startDate = date[0].startOf('day').toISOString(true);
                                    }

                                    if(date[1]) {
                                        endDate = date[1].endOf('day').toISOString(true);
                                    }
                                }

                                onChange(startDate, endDate);
                            }}
                        />
                    </div>
                </div>
            </div>
            <style>
                {`
                  .custom-filter-dropdown .date-wrapper {
                    display: block;
                    overflow: visible;
                    position: relative;
                    line-height: 0;
                   }
                `}
            </style>
        </>
    );
};
