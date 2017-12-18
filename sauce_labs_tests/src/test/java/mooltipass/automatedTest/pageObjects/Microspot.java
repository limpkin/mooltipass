package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Microspot extends AbstractPage{
	
	public Microspot (WebDriver driver) {
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
	
	@FindBy(xpath = "(//button[.//span[contains(text(),'Abmelden')]])[2]")
	private WebElement logoutBtn;
	



	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}
	
	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);		
		Actions action = new Actions(driver);
		action.moveToElement(loginBtn);
		action.perform();
	}
	
	public void submit(){
	submitLogin.click();
	}
	
	public void logout(){
		waitUntilAppears(logoutBtn);
		logoutBtn.click();
		sleep(3000);
	}
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//button[.//span[contains(text(),'Abmelden')]]"));
		return isElementPresent(By.xpath("//button[.//span[contains(text(),'Abmelden')]]"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("email"));
	}
	
}
