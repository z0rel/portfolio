import React from "react";
import {routing} from "../routes/routing";
import site from "../constants/siteConstants";
import {imgIconWh} from "../utils/responsiveWh";
import {UndefinedImgParams} from "../utils/exceptions";


let IconAlt = ({
                      alt,
                      className,
                      icon,
                      // iconAlt, // changed to widthAndHeight
                      title,
                      imgParams, // changed from <img {...getImgWh(routing.path_sprites_png(iconAlt))}
                      styles
                  }) => {
    if (imgParams === undefined) {
        throw new UndefinedImgParams(`undefinedImgParams in IconAlt: icon:${icon}`)
    }
    return <>
        <svg className={`${className} ${styles.svg}`}>
            <use xlinkHref={`#s-ic-${icon}`} {...title}/>
        </svg>
        {
            site.data.config.noscript_enabled
                ? <noscript>
                    <img {...imgIconWh(icon, imgParams)} className={`${className} ${styles.svg}`} alt={alt} />
                </noscript>
                : <></>
        }
    </>
}

export default IconAlt;