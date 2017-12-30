package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Microspot extends AbstractPage {

	public Microspot(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "email")
	private WebElement email;

	@FindBy(id = "password")
	private WebElement password;

	@FindBy(xpath = "//button[.//span[contains(text(),'Anmelden')]]")
	private WebElement submitLogin;

	@FindBy(xpath = "//span[text()='Mein Konto']")
	private WebElement loginBtn;

	@FindBy(xpath = "(//button[contains(@class,'Xp')])[1]")
	private WebElement logoutBtn;

	@FindBy(xpath = "//span[text()='Mein Konto']")
	private WebElement logoutMenu;

	@FindBy(xpath = "//span[contains(text(),'Suchen')]")
	private WebElement searchBar;

	public void enterEmail(String value) {
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value) {

		waitUntilAppears(password);
		password.sendKeys(value);
	}

	public void goToLogin() {
		if (isElementPresent(By.xpath("//buttton[contains(@class,'Xpqhts')]"))) {
			waitUntilNotVisible(By.xpath("//buttton[contains(@class,'Xpqhts')]"));
		}

		waitUntilAppears(loginBtn);
		Actions action = new Actions(driver);
		action.moveToElement(loginBtn);
		action.perform();

	}

	public void submit() {
		submitLogin.click();
	}

	public void logout() {

		waitUntilClickable(logoutMenu);
		logoutMenu.click();
		waitUntilAppears(logoutBtn);
		logoutBtn.click();

	}

	public boolean checkLogin() {

		waitUntilAppears(By.xpath("//button[.//span[contains(text(),'Abmelden')]]"));
		return isElementPresent(By.xpath("//button[.//span[contains(text(),'Abmelden')]]"));
	}

	public boolean checkAtLoginPage() {
		waitUntilAppears(By.id("email"));
		return isElementPresent(By.id("email"));
	}

}
