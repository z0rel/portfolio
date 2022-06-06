import React, {useContext, useEffect, useState} from "react";
import Link from "next/link";
import {
    getDataLazy,
    getImageFnameLazy,
    getRespImgWh,
} from "../utils/responsiveWh";
import { mkOrEmpty } from "../utils/mk";
import {
    getImgAttrsSrcset,
    getImgFnameProperty,
    getSourceAttrsJpeg,
    getSourceAttrsWebp,
} from "../responsive/setupSrcset";
import {GlobalContext} from "../utils/GlobalContext";
import { LazyLoadImage } from "react-lazy-load-image-component";

const PortfolioMainImg = ({
                              work,
                              image,
                              rawImgClassName,
                              genSrcsetJpg,
                              genSrcsetWebp,
                              sizesStr,
                              styles,
                              imgParams,
                          }) => {

    const globalCtx = useContext(GlobalContext);

    const [showedState, setShowedState] = useState(false)
    useEffect(() => {
        console.log(globalCtx.webpack)
        setShowedState(true)
    }, [])


    let imgClassName = rawImgClassName
        ? mkOrEmpty(`i-${rawImgClassName}`, styles)
        : "";

    let imgSrcsetArrs = getImgFnameProperty(
        getImageFnameLazy(work.uri_rel, image), genSrcsetJpg, genSrcsetWebp
    );
    imgSrcsetArrs.sizesStr = sizesStr;
    let imgScriptClasses = `${mkOrEmpty(
        "lazy",
        styles
    )} ${imgClassName} ${mkOrEmpty(image.speclassmain, styles)}`
    let widthAndHeight = getDataLazy(work.uri_rel, image, imgParams);

    return (
        <Link href={`${work.uri}${image.file}${image.ext}`}>
            <a className={styles[rawImgClassName]} href={`${work.uri}${image.file}${image.ext}`}>
                {
                    <img
                        height={image.height}
                        src={image.src} // use normal <img> attributes as props
                        width={image.width}
                        {...(globalCtx.webpack ? getSourceAttrsWebp(imgSrcsetArrs) : getSourceAttrsJpeg(imgSrcsetArrs))}
                        {...widthAndHeight}
                        className={imgScriptClasses}
                        alt={image.seo_alt_descr_main?.trim()}
                        title={image.seo_alt_title_main?.trim()}
                    />
                    /*
                    !showedState
                        ? (
                            <picture>
                                <img
                                    className={imgScriptClasses}
                                    {...widthAndHeight}
                                    alt={image.seo_alt_descr_main?.trim()}
                                    title={image.seo_alt_title_main?.trim()}
                                />
                            </picture>
                        )
                        : (
                            <img
                                className={imgScriptClasses}
                                {...getImgAttrsSrcset(imgSrcsetArrs)}
                                {...widthAndHeight}
                                alt={image.seo_alt_descr_main?.trim()}
                                title={image.seo_alt_title_main?.trim()}
                            />



                             <picture>
                                <source {...getSourceAttrsWebp(imgSrcsetArrs)} />
                                <source {...getSourceAttrsJpeg(imgSrcsetArrs)} />
                                <img
                                    className={imgScriptClasses}
                                    {...getImgAttrsSrcset(imgSrcsetArrs)}
                                    {...widthAndHeight}
                                    alt={image.seo_alt_descr_main?.trim()}
                                    title={image.seo_alt_title_main?.trim()}
                                />
                            </picture>
                        )
                    */}
                <noscript>
                    <img
                        className={imgClassName}
                        {...getRespImgWh(work.uri_rel, image, imgParams)}
                        alt={image.seo_alt_descr_main?.trim()}
                        title={image.seo_alt_title_main?.trim()}
                    />
                </noscript>
            </a>
        </Link>
    );
};

export default PortfolioMainImg;
