#let project(title: "", authors: (), logo: none, body) = {
  // Set the document's basic properties.
  set document(author: authors.map(a => a.name), title: title)
  set page(numbering: "1", number-align: center)
  set text(font: "New Computer Modern", lang: "en")
  show math.equation: set text(weight: 400)
  set heading(numbering: "1.1.")

  // Title page.
  // The page can contain a logo if you pass one with `logo: "logo.png"`.
  v(0.6fr)
  if logo != none {
    align(right, image(logo, width: 26%))
  }
  v(9.6fr)

  text(2em, weight: 700, title)

  // Author information.
  pad(
    top: 0.7em,
    right: 20%,
    grid(
      columns: (1fr,) * calc.min(2, authors.len()),
      gutter: 1em,
      ..authors.map(author => align(start)[
        *#author.name* \
        #author.email \
        #author.affiliation
      ]),
    ),
  )

  v(2.4fr)
  pagebreak()


  // Main body.
  set par(justify: true)

  body
}

#show: project.with(
  title: "IFJ23",
  authors: (
    (name: "Jakub Kloub - 25%", email: "xkloub03 - vedouci", affiliation: ""),
    (name: "Matúš Moravčík - 25%", email: "xmorav48", affiliation: ""),
    (name: "Le Duy Nguyen - 25%", email: "xnguye27", affiliation: "Lexical Analysis Symbol Table"),
    (name: "Lukáš Habr - 25%", email: "xhabrl01", affiliation: ""),
  ),
)

#include "lexical_analysis.typ"
#pagebreak()
