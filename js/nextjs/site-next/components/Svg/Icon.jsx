import React from "react";
import site from "../constants/siteConstants";
import {imgIconWh} from "../utils/responsiveWh";
import {UndefinedImgParams} from "../utils/exceptions";

let SpanCover = ({spanCover, tags, children}) => {
    return spanCover
        ? <span className={spanCover} {...tags} >{children}</span>
        : <>{children}</>
}

let Icon = ({
                alt,
                attrs,
                className,
                icon,
                spanCover,
                style,
                tags,
                title,
                imgParams, // changed from <img {...getImgWh(routing.path_sprites_png(icon))}
                styles
            }) => {
    let styleObj = style ? {style: style} : {}
    let classStr = `${className ? className : ''}${className ? ' ' : ''}${styles.svg}`;
    if (imgParams === undefined) {
        throw new UndefinedImgParams(`undefinedImgParams in Icon: icon:${icon} alt: ${alt}`)
    }

    return <SpanCover spanCover={spanCover} tags={tags}>
        <svg {...attrs} {...styleObj} className={classStr}>
            <use xlinkHref={`#s-ic-${icon}`} {...title} />
        </svg>
        {
            site.data.config.noscript_enabled
                ?
                <noscript>
                    <img
                        {...imgIconWh(icon, imgParams)}
                        {...styleObj}
                        className={classStr}
                        alt={alt}
                    />
                </noscript>
                : <></>
        }
    </SpanCover>
}

export default Icon;