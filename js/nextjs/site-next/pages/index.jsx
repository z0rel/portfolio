import React, {useEffect} from "react";

import styles from '../scss/Index.module.scss'
import setupSvgLocalstorage from "../components/utils/setupSvgLocalstorage";

import NavbarRow from "../components/Head/NavbarRow";
import Jumbotron from "../components/pages/index/Jumbotron";
import ServicesSection from "../components/pages/index/ServicesSection";
import AboutCompany from "../components/pages/index/AboutCompany";
import HowWeWorks from "../components/pages/index/HowWeWorks";
import ContactsForm from "../components/pages/index/ContactsForm";
import ContactsData from "../components/pages/index/ContactData";
import Footer from "../components/Head/Footer";
import PortfolioMain from "../components/pages/index/PortfolioMain";


export async function getStaticProps(context) {
    const loadStaticData = (await import("../components/pages/index/utils/loadStaticData")).default;
    return await loadStaticData();
}


const Home = ({imgParams}) => {
    useEffect(() => {
        setupSvgLocalstorage(window, document, '/img/main.svg', '00')
    }, [])


    return (
        <>
            <header className={styles.parallaxGroup}>
                <div
                    className={[styles.contentHead, styles.main, styles.parallaxLayer, styles.parallaxLayerBack].join(' ')}></div>
                <div
                    className={[styles.contentHead, styles.main, styles.parallaxLayer, styles.parallaxLayerBase].join(' ')}>
                    <div className={styles.container}>
                        <NavbarRow is_index={true} imgParams={imgParams} styles={styles}/>
                        <Jumbotron styles={styles}/>
                    </div>
                </div>
            </header>
            <ServicesSection styles={styles} imgParams={imgParams} />
            <AboutCompany styles={styles} imgParams={imgParams} />
            <HowWeWorks styles={styles} />
            <PortfolioMain styles={styles} imgParams={imgParams} />
            <ContactsData styles={styles} imgParams={imgParams}/>
            <ContactsForm styles={styles} imgParams={imgParams} />
            <Footer styles={styles} imgParams={imgParams} />
        </>
    )
}

export default Home
