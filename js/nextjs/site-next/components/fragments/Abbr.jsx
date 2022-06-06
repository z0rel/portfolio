const Abbr = ({descr, abbr}) => {
    return <span class='abbr'
                 data-toggle="tooltip"
                 data-placement="top"
                 title={descr}
                 dangerouslySetInnerHTML={{__html: abbr}}></span>
}
export default Abbr;