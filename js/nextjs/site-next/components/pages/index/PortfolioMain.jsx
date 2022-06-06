import React, { useEffect } from "react";
import { mk } from "../../utils/mk";
import site from "../../constants/siteConstants";
import PortfolioWork from "./utils/PortfolioWork";
import { Swiper, SwiperSlide} from "swiper/react";


// Debounced values
import { useWindowSize } from '@react-hook/window-size';

// import Swiper core and required modules
import SwiperCore, { Pagination, Navigation } from "swiper";

// install Swiper modules
SwiperCore.use([Pagination, Navigation]);

let breakpoints = {
    xs: 0,
    sm: 576,
    md: 768,
    lg: 992,
    xl: 1200
};

const PortfolioMain = ({ styles, imgParams }) => {
    let {width, height} = useWindowSize({wait: 500});

    useEffect(() => {
        if (typeof window !== "undefined") {
            // handleContentLoaded(styles);
            // let selector = `.${styles['owl-carousel']}`;
            // console.log(selector)
            // $(selector).owlCarousel();
        }
    }, []);

    let responsiveItemsSm = 1;
    let classMobile = 'on-mobile';

    if ((width >= height && height < 500) || (width < height && width < 500))
    {
        responsiveItemsSm = 1;
        classMobile = 'on-mobile';
    }
    else {
        classMobile = 'not-on-mobile';
        responsiveItemsSm = 2;
    }


    let portfolio_works = Object.values(site.data.portfolio.works)
        .filter((item) => item.work.display_main)
        .map((item) => item.work);
    portfolio_works.sort((l, r) => l.main_order - r.main_order);

    // let work = portfolio_works[0];
    // return <PortfolioWork work={work} index={0} styles={styles} imgParams={imgParams} />
    return (
        <section className={mk("container portfolio-section", styles)}>
            <h1 id={site.data.nav.portfolio.anchor_}>Выполненные проекты</h1>
            <div className={mk(`portfolio-carousel-container ${classMobile}`, styles)}>
                <Swiper
                    slidesPerView={3}
                    spaceBetween={0}
                    slidesPerGroup={1}
                    loop={true}
                    loopFillGroupWithBlank={true}
                    pagination={{
                        clickable: true,
                    }}
                    navigation={true}
                    className="mySwiper"
                    // Responsive breakpoints
                    breakpoints={{
                        // when window width is >= 320px
                        [breakpoints["xs"]]: {
                            slidesPerView: 1,
                        },
                        // when window width is >= 480px
                        [breakpoints["sm"]]: {
                            slidesPerView: responsiveItemsSm,
                            spaceBetween: (responsiveItemsSm === 1 ? 0 : 20)
                        },
                        [breakpoints["md"]]: {
                            slidesPerView: 2,
                            spaceBetween: 0
                        },
                        [breakpoints["lg"]]: {
                            slidesPerView: 3,
                            spaceBetween: 0
                        },
                        [breakpoints["xl"]]: {
                            slidesPerView: 3,
                            spaceBetween: 0
                        },
                    }}
                >
                    {
                        portfolio_works.map((work, index) => (
                            <SwiperSlide key={index}>
                                <PortfolioWork
                                    work={work}
                                    index={index}
                                    styles={styles}
                                    imgParams={imgParams}
                                />
                            </SwiperSlide>
                        ))
                    }
                </Swiper>
                <div
                    id={styles["portfolioCarousel"]}
                    className={mk("owl-carousel owl-theme", styles)}
                >
                    {/*portfolio_works.map((work, index) => (
            <PortfolioWork
              work={work}
              index={index}
              styles={styles}
              imgParams={imgParams}
            />
          ))*/}
                </div>
            </div>
            {/*
                <div className={styles["show-more-all-wrapper-1"]}>
                    <div className={mk("show-more-all-wrapper noscript-hidden", styles)}>
                        <button type="button" className={mk("show-more-all btn btn-link", styles)}>
                        <span className={styles["showhide"]}>
                            <span className={styles["wshow"]}>Показать все работы</span>
                            <span className={styles["whide"]}>Свернуть список работ</span>
                            <span className={styles["down"]}></span>
                            <span className={styles["up"]}></span>
                        </span>
                        </button>
                    </div>
                </div>

                 */}
        </section>
    );
};

export default PortfolioMain;
