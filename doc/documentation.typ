#let project(title: "", year: "", authors: (), logo: none, team: "", extensions: (), body) = {
  // Set the document's basic properties.
  set document(author: authors.map(a => a.name), title: title)
  set page(numbering: "1", number-align: center)
  set text(font: "New Computer Modern", lang: "en")
  show math.equation: set text(weight: 400)
  set heading(numbering: "1.1.")

  v(0.6fr)
  if logo != none {
    align(center, image(logo, width: 100%))
  }

  v(1fr)

  align(center)[
      #text(2em, weight: 700, title) \
      #text(1.5em, year)

      #v(0.4fr)
      #text(1.2em, team)
      #v(0.6fr)

      // Author information.
      #pad(
        top: 0.7em,
        right: 20%,
        left: 20%,
        grid(
          columns: (1fr,) * calc.min(2, authors.len()),
          gutter: 3em,
          ..authors.map(author => align(center)[
            *#author.name* - #author.distribution \
            #author.xlogin \
            #author.works.join("\n")
          ]),
        ),
      )

      #v(0.4fr)
      Extensions: #extensions.join(", ")
  ]

  v(2.4fr)
  pagebreak()


  // Main body.
  set par(justify: true)

  body
}


#show: project.with(
  title: "Formal Languages and Compilers",
  year: "2023-2024",
  logo: "img/fit_logo.png",
  authors: (
    (name: "Jakub Kloub", distribution: 28%, xlogin: "xkloub03 - vedoucí", works: ("Recursive descent", "Symbol Table")),
    (name: "Matúš Moravčík", distribution: 28%, xlogin: "xmorav48", works: ("Precedence Analysis",)),
    (name: "Le Duy Nguyen", distribution: 29%, xlogin: "xnguye27", works: ("Lexical Analysis", "Symbol Table")),
    (name: "Lukáš Habr", distribution: 15%, xlogin: "xhabrl01", works: ("Code Generation",)),
  ),
  team: "Tým xkloub03, varianta vv-BVS",
  extensions: ("BOOLTHEN", "FUNEXP"),
)

#include "info.typ"
#pagebreak()
#include "lexical_analysis.typ"
#pagebreak()
#include "syntactic_analysis.typ"
#pagebreak()
#include "semantic_analysis.typ"
#pagebreak()
#include "code_generation.typ"