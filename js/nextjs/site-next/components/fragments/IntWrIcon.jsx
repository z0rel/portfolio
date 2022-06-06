import React from "react";
import Icon from "../Svg/Icon";

const IntWrIcon = ({subclass, icon, widthAndHeight}) => {
    return <span class="int-icon-wr">
        <Icon span-cover="li-cover" className={`icon-our-work ${subclass} s-${ icon }`} icon={icon} widthAndHeight={widthAndHeight} />
    </span>
}

export default IntWrIcon