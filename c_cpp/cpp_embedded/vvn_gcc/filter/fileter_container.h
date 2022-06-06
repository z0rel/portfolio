/*
 * IncFile1.h
 *
 * Created: 18.03.2022 17:19:27
 *  Author: artem
 */


#ifndef CALMAN_FILTER_CONTAINER_H_
#define CALMAN_FILTER_CONTAINER_H_


#include "CalmanFilter.h"

namespace calman {


class CalmanContainer {
  public:
    CalmanFilter viscosity;
    CalmanFilter dencity;
    CalmanFilter temperature;

    CalmanContainer() {
		viscosity.init(1024);
		dencity.init(1024);
		temperature.init(1024);
	}
};

extern CalmanContainer calmanContainer;

} // namespace calman


#endif /* INCFILE1_H_ */
