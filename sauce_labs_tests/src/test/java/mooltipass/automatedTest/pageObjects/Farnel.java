package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

//There is a problem with, Since Farnel ch is a subdomain, Mooltipass doesnot know 
//which password to store
public class Farnel  extends AbstractPage{
	public Farnel (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "logonId")
	private WebElement email;

	@FindBy(id = "logonPassword")
	private WebElement password;

	@FindBy(xpath = "//input[@value='Anmelden']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//p[@id='guestPar']//a[contains(text(),'Anmelden')]")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//a[@href='/webapp/wcs/stores/servlet/Logoff?myAcctMain=1&URL=LogonForm']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//a[contains(text(),'Weiter')]")
	private WebElement weiterBtn;

	@FindBy(xpath = "//div[@class='my-account-espot menu-dropdown']")
	private WebElement dashBoard;


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
		clickWithAction(loginBtn);
	}
	public void clickContinue()
	{
		
		waitUntilAppears(weiterBtn);		
		weiterBtn.click();
		sleep(2000);
	}
	public void submit(){
	submitLogin.click();
	}
	
	public void clickDashboard()
	{
		dashBoard.click();
		
	}
	public void logout(){
		waitUntilAppears(logoutBtn);
		clickWithAction(logoutBtn);
	}
	public boolean checkLogin(){

		//waitUntilAppears(By.id("loggedInPar"));
		return isElementPresent(By.id("loggedInPar"));
	}
	public boolean checkAtLoginPage(){
		waitUntilAppears(By.id("logonId"));
		return isElementPresent(By.id("logonId"));
	}
}
