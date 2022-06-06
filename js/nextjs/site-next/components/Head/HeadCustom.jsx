import Head from "next/head";
import React from "react";
import site from "../constants/siteConstants";
import getFavicon from "../utils/getFavicon";

const SITE = site.domains;

let HeadCustom = ({page, children}) => {
    let comment = `<!--[if IE]><link type='image/x-icon' rel="icon" href="${getFavicon()}" sizes="48x48"/><![endif]-->`;
    let owlRaw = '.owl-carousel { display: flex; }'

    return <Head>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
        <meta content="width=device-width, initial-scale=1" name="viewport"/>
        <title>{ page.pageTitle }</title>
        <meta content="IE=edge" http-equiv="X-UA-Compatible"/>
        <link rel="icon" type='image/png' sizes="180x180" href={`${SITE.domain_canonical}favicon.png`} />
        <link rel="apple-touch-icon" type='image/png' sizes="180x180" href={`${SITE.domain_canonical}innostan-apple-touch-icon.png`} />
        {/* <meta name="react-comment-hack" dangerouslySetInnerHTML={{__html: comment}} /> */}

        <meta name="description" content={page.description}/>
        <meta http-equiv="X-UA-Compatible" content="IE=edge"/>
        <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1"/>

        {/* Template Basic Images Start */}

        {/* Twitter and Facebook OpenGraph Metadata */}
        <meta property="og:title" content={ page.ogTitle } />
        <meta property="og:description" content={page.ogDescription}/>
        <meta property="og:image" content={`${SITE.domain_robots}${page.ogImage}`} />
        <meta property="vk:image" content={`${SITE.domain_robots}${page.ogImage}`} />
        <meta property="og:url" content={`${SITE.domain_human_seo}${page.pageUrl}`} />
        <meta property="og:type" content="website" />

        {
            page.twitterCardType
                ? <>
                    <meta name="twitter:card" content={ page.twitterCardType }/>
                    {/*meta name="twitter:card" content="summary" / */}
                </>
                : <></>
        }
        {/* 	<meta name="twitter:creator" content="@webholt"> */}

        <meta name="twitter:title" content={ page.ogTitle } />
        <meta name="twitter:description" content={ page.ogDescription } />
        <meta name="twitter:image" content={`${SITE.domain_robots}${page.ogImage}`} />

        <link href={`${ SITE.domain }${page.pageUrl}`} rel="canonical" />

        <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
        {/* Template Basic Images End */}

        {/* Custom Browsers Color Start */}
        <meta name="theme-color" content="#000"/>
        {/* Custom Browsers Color End */}
        <meta name="format-detection" content="telephone=no"/>

        {children}
        <noscript>
            <style>{owlRaw}</style>
            <link rel="stylesheet" property="stylesheet" type="text/css" href="/css/main_noscript.min.css" />
        </noscript>
    </Head>
}

export default HeadCustom;