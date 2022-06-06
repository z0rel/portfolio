import site from "../../../constants/siteConstants";
import getImagesParams from "../../../constants/getImagesParams";
import {routing} from "../../../routes/routing";

const loadStaticData = async (context) => {
    let portfolioWorks = (Object.values(site.data.portfolio.works)
            .filter(item => item.work.display_main)
            .map((item) => item.work)
    );
    let portfolio_works_horizontal = (portfolioWorks
            .filter(work => work.horisontal_maket === 1)
    )
    let hrefByImage = (work, image) => `${ work.uri_rel }${image.file}${image.ext}`;
    let portfolio_works_images = [
        ...portfolioWorks.map(work => hrefByImage(work, work.img_main)),
        ...portfolioWorks.map(work => hrefByImage(work, work.img_1)),
        ...portfolioWorks.map(work => hrefByImage(work, work.img_2)),
        ...portfolioWorks.map(work => hrefByImage(work, work.img_3)),
        ...portfolio_works_horizontal.map(work => hrefByImage(work, work.img_4))
    ];

    let imgParams = getImagesParams([
        site.data.nav.logo.internal,
        site.data.nav.logo.footer,
        site.data.nav.s_repair.noscript_png,
        site.data.nav.s_modernization.noscript_png,
        site.data.nav.s_shipping.noscript_png,
        site.data.nav.s_integration.noscript_png,
        'stankostroitelnyj-zavod-innostan-logo-internal.png',
        ...[
            'intelligence-img', 'computer-vision', 'networks', 'automation-img', 'applications',
            'repair-middle', 'shipping-start', 'robot', 'controller', 'intelligence-ic', 'networks',
            'automation-ic', 'production', 'repair', 'code', 'phone', 'mail', 'person', 'mail', 'edit',
            'logo-innostan', 'phone',
        ].map(x => routing.path_sprites_png(x)),
        ...portfolio_works_images
    ])

    return {
        props: {imgParams: imgParams}, // will be passed to the page component as props
    }
}

export default loadStaticData;