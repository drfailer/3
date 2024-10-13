#ifndef BUILTIN_H
#define BUILTIN_H
#include "typednodes.hpp"
#include <memory>

/******************************************************************************/
/*                                     IO                                     */
/******************************************************************************/

/**
 * @brief  Gestion de l'affichage. On ne peut afficher que soit une chaine de
 *         caratères (`"str"`), soit une variable.
 */
class Print : public Node {
  public:
    Print(std::shared_ptr<Node> content) : str_(""), content_(content) {}

    Print(std::string str)
        : str_(str), content_(std::shared_ptr<Node>(nullptr)) {}

    void compile(std::ofstream &, int) override;
    void display() override;

  private:
    std::string str_;
    std::shared_ptr<Node> content_ = nullptr;
};

/**
 * @brief  Gestion de la saisie clavier.
 */
class Read : public Node {
  public:
    Read(std::shared_ptr<TypedNode> variable) : variable_(variable) {}

    void display() override;
    void compile(std::ofstream &, int) override;

  private:
    std::shared_ptr<TypedNode> variable_ = nullptr;
};

#endif
