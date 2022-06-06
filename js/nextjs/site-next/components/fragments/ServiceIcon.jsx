import React from "react";
import microdata from "../microdata/microdata";
import IconServiceSvg from "../Svg/IconServiceSvg";
import site from "../constants/siteConstants";
import {mk} from "../utils/mk";
import Link from "next/link";


const ServiceIcon = ({styles, incl_data, notitle, imgParams}) => {
   //IconServiceSvg:    png_noscript=incl_data.noscript_png
	let fname = incl_data.noscript_png;
	return <li className={styles["service-custom"]} {...microdata.ITEM_LIST_OFFER}>
		<Link href={ incl_data.url } passHref>
			<a {...microdata.ITEM_OFFERED_SERVICE} >
				<div className={mk(`service-custom-img subblock align-top ${ incl_data.className }`, styles)}>
					<IconServiceSvg fname={fname}
									imgParams={imgParams}
									className={styles[site.data.nav.service_class]}
									icon={incl_data.classIcon}
									alt={incl_data.title}
									styles={styles}
					/>
				</div>
				{
					!notitle
						? <div className={mk("subblock text", styles)}
							   itemProp="name"
							   dangerouslySetInnerHTML={{__html: incl_data.title}}></div>
						: <></>
				}
			</a>
		</Link>
	</li>
}

export default ServiceIcon