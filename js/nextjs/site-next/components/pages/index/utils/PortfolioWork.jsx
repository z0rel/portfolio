import React from "react";
import { mk } from "../../../utils/mk";
import PortfolioMainImg from "../../../Portfolio/PortfolioMainImg";
import Link from "next/link";
import {
    sizes_1,
    sizes_1_horiz,
    sizes_1_horiz_str_jpg,
    sizes_1_horiz_str_webp,
    sizes_1_str_jpg,
    sizes_1_str_webp,
    sizes_2,
    sizes_2_horiz,
    sizes_2_horiz_str_jpg,
    sizes_2_horiz_str_webp,
    sizes_2_str_jpg,
    sizes_2_str_webp,
    sizes_main,
    sizes_main_horiz,
    sizes_main_horiz_str_jpg,
    sizes_main_horiz_str_webp,
    sizes_main_str_jpg,
    sizes_main_str_webp,
} from "../../../responsive/setupSrcset";

const PortfolioWork = ({ work, index, styles, imgParams }) => {
    return (
        <div className={styles["item"]} key={index}>
            <article className={styles["portfolio-item"]}>
                {work.horisontal_maket !== 1 ? (
                    <div className={mk(`work-images v ${work.class_id}`, styles)}>
                        <PortfolioMainImg
                            work={work}
                            image={work.img_main}
                            rawImgClassName="img-main"
                            styles={styles}
                            imgParams={imgParams}
                            genSrcsetJpg={sizes_main_str_jpg}
                            genSrcsetWebp={sizes_main_str_webp}
                            sizesStr={sizes_main}
                        />
                        <div className={styles["img-123"]}>
                            <PortfolioMainImg
                                work={work}
                                image={work.img_1}
                                rawImgClassName="img-1"
                                styles={styles}
                                imgParams={imgParams}
                                genSrcsetJpg={sizes_1_str_jpg}
                                genSrcsetWebp={sizes_1_str_webp}
                                sizesStr={sizes_1}
                            />
                            <div className={styles["img-23"]}>
                                <PortfolioMainImg
                                    work={work}
                                    image={work.img_2}
                                    rawImgClassName="img-2"
                                    styles={styles}
                                    imgParams={imgParams}
                                    genSrcsetJpg={sizes_2_str_jpg}
                                    genSrcsetWebp={sizes_2_str_webp}
                                    sizesStr={sizes_2}
                                />
                                <PortfolioMainImg
                                    work={work}
                                    image={work.img_3}
                                    rawImgClassName="img-3"
                                    styles={styles}
                                    imgParams={imgParams}
                                    genSrcsetJpg={sizes_2_str_jpg}
                                    genSrcsetWebp={sizes_2_str_webp}
                                    sizesStr={sizes_2}
                                />
                            </div>
                        </div>
                    </div>
                ) : (
                    <div
                        className={mk(
                            `work-images horisontal-maket ${work.class_id}`,
                            styles
                        )}
                    >
                        <div
                            className={mk(
                                `img-01${work.horisontal_maket_2horiz === 1 ? " horiz-2" : ""}`,
                                styles
                            )}
                        >
                            <PortfolioMainImg
                                work={work}
                                image={work.img_main}
                                rawImgClassName="img-main"
                                styles={styles}
                                imgParams={imgParams}
                                genSrcsetJpg={sizes_main_horiz_str_jpg}
                                genSrcsetWebp={sizes_main_horiz_str_webp}
                                sizesStr={sizes_main_horiz}
                            />
                            <PortfolioMainImg
                                work={work}
                                image={work.img_1}
                                rawImgClassName="img-1"
                                styles={styles}
                                imgParams={imgParams}
                                genSrcsetJpg={sizes_1_horiz_str_jpg}
                                genSrcsetWebp={sizes_1_horiz_str_webp}
                                sizesStr={sizes_1_horiz}
                            />
                        </div>
                        <div className={styles["img-234"]}>
                            <PortfolioMainImg
                                work={work}
                                image={work.img_2}
                                rawImgClassName="img-2"
                                styles={styles}
                                imgParams={imgParams}
                                genSrcsetJpg={sizes_2_horiz_str_jpg}
                                genSrcsetWebp={sizes_2_horiz_str_webp}
                                sizesStr={sizes_2_horiz}
                            />
                            <PortfolioMainImg
                                work={work}
                                image={work.img_3}
                                rawImgClassName="img-3"
                                styles={styles}
                                imgParams={imgParams}
                                genSrcsetJpg={sizes_2_horiz_str_jpg}
                                genSrcsetWebp={sizes_2_horiz_str_webp}
                                sizesStr={sizes_2_horiz}
                            />
                            <PortfolioMainImg
                                work={work}
                                image={work.img_4}
                                rawImgClassName="img-4"
                                styles={styles}
                                imgParams={imgParams}
                                genSrcsetJpg={sizes_2_horiz_str_jpg}
                                genSrcsetWebp={sizes_2_horiz_str_webp}
                                sizesStr={sizes_2_horiz}
                            />
                        </div>
                    </div>
                )}
                <div className={styles["textinfo"]}>
                    <h2 className={styles["head"]}>
            <span className={styles["row1"]}>
              <span className={styles["head-title"]}>
                <Link href={work.uri} passHref>
                  <a>
                    <span
                        className={styles["mhead"]}
                        dangerouslySetInnerHTML={{ __html: work.machine_head }}
                    /> <span
                        className={styles["title"]}
                        dangerouslySetInnerHTML={{ __html: work.title }}
                    />
                  </a>
                </Link>
              </span>
              <span className={styles["year"]}>{work.year}</span>
            </span>
                        <span
                            className={styles["mtype"]}
                            dangerouslySetInnerHTML={{ __html: work.machine_type }}
                        ></span>
                    </h2>
                    <p
                        className={styles["descr"]}
                        dangerouslySetInnerHTML={{ __html: work.description }}
                    ></p>
                    <div className={styles["read-more"]}>
                        <Link href={work.uri} passHref>
                            <a
                                className={mk("read-more-btn btn btn-primary", styles)}
                                role="button"
                            >
                                ПОДРОБНЕЕ
                            </a>
                        </Link>
                    </div>
                </div>
            </article>
        </div>
    );
};
export default PortfolioWork;
