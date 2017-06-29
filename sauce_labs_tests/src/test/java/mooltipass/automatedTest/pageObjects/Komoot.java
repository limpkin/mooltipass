package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Komoot extends AbstractPage{

	public Komoot(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	
	@FindBy(xpath = "//a[text()='Anmelden']")
	private WebElement loginBtn;

	@FindBy(xpath = "//input[@id='email']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='password']")
	private WebElement password;

	@FindBy(xpath = "//button[text()='Anmelden']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[text()='Abmelden']")
	private WebElement logoutBtn;
	
	@FindBy(className = "c-thumbnail__img")
	private WebElement user;
	
	public void goToLogin(){
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		submitLogin.click();
		}
	
	public boolean checkLogin(){

		waitUntilAppears(By.className("c-thumbnail__img"));
		return isElementPresent(By.className("c-thumbnail__img"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@id='login_email']"));
	}
	
	public void logout(){
		Actions action = new Actions(driver);
		action.moveToElement(user);
		action.perform();
		logoutBtn.click();

	}
}