import React, {useContext} from "react";
import {mk} from "../utils/mk";
import site from "../constants/siteConstants";
import {imgWh} from "../utils/responsiveWh";
import {UndefinedImgParams} from "../utils/exceptions";

let IconServiceSvg = ({
                          alt,
                          attrs,
                          className,
                          icon,
                          noscrTags,
                          // pngNoscript, changed to widthAndHeight
                          title,
                          fname,
                          imgParams,
                          styles
                      }) => {
    if (imgParams === undefined) {
        throw new UndefinedImgParams(`undefinedImgParams in IconServiceSvg: icon:${icon}`)
    }
    return <>
        <svg {...attrs} className={`${className} ${mk('svg noscript-hidden', styles)}`}>
            <use xlinkHref={`#s-ic-${icon}`} {...title}/>
        </svg>
        {
            site.data.config.noscript_enabled
                ? <noscript>
                    <img {...imgWh(fname, imgParams)}
                         className={`${className} ${styles.svg}`}
                         alt={alt}
                         {...noscrTags}
                    />
                </noscript>
                : <></>
        }
    </>
}

export default IconServiceSvg;