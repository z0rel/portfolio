import React from "react";
import site from "../constants/siteConstants";
import {mk} from "../utils/mk";
import {imgIconWh, imgWh} from "../utils/responsiveWh";
import {UndefinedImgParams} from "../utils/exceptions";

let IconLogo = ({
                       alt,
                       className,
                       //fileNoscrpit, // changed to widhtAndHeight
                       icon,
                       style,
                       title,
                       imgParams,
                       specIconFname,
                       styles
                   }) => {
    if (imgParams === undefined) {
        throw new UndefinedImgParams(`undefinedImgParams in IconLogo: icon:${icon}`)
    }
    let params = specIconFname ? imgWh(specIconFname, imgParams) : imgIconWh(icon, imgParams);
    return <>
        <svg className={`${className} ${mk("svg noscript-hidden", styles)}`} style={style} >
            <use xlinkHref={`#s-ic-${icon}`} xlinkTitle={title}/>
        </svg>
        {
            site.data.config.noscript_enabled
                ? <noscript>
                    <img {...params} className={`${className} ${styles.svg}`} alt={alt} style={style} />
                </noscript>
                : <></>
        }
    </>
}

export default IconLogo